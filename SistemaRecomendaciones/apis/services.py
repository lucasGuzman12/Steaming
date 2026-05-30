from __future__ import annotations

from collections import Counter
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import secrets
import sqlite3
from typing import Any, Optional

import requests

from .data_manager import (
    APISettings,
    ConfigurationError,
    DataManager,
    ServiceError,
)


class SteamUsuarioManager(DataManager):
    def __init__(self, settings: APISettings, session=None) -> None:
        super().__init__(
            api_gestion="steam_usuario",
            link="https://api.steampowered.com/IPlayerService/GetOwnedGames/v1/",
            key=settings.steam_api_key,
            session=session,
            timeout=settings.timeout,
        )
        self.settings = settings

    def get(self, steam_id: str, enrich_top: int = 5) -> dict[str, Any]:
        self._ensure_key("STEAM_API_KEY")
        params = {
            "key": self.key,
            "steamid": steam_id,
            "include_appinfo": True,
            "include_played_free_games": True,
            "format": "json",
        }
        data = self._request_json("GET", self.link, params=params)
        response_data = data.get("response", {})
        raw_games = response_data.get("games", [])
        accessible = "games" in response_data

        juegos = []
        for game in sorted(
            raw_games,
            key=lambda item: item.get("playtime_forever", 0),
            reverse=True,
        ):
            juegos.append(
                {
                    "appid": game.get("appid"),
                    "nombre": game.get("name", "Sin nombre"),
                    "minutos_jugados": game.get("playtime_forever", 0),
                    "horas_jugadas": round(
                        game.get("playtime_forever", 0) / 60,
                        2,
                    ),
                }
            )

        for juego in juegos[: max(0, enrich_top)]:
            try:
                juego.update(self._get_game_detail(juego["appid"]))
            except ServiceError as exc:
                juego["detalle_error"] = str(exc)

        payload = {
            "api": self.api_gestion,
            "steam_id": steam_id,
            "cantidad": len(juegos),
            "perfil_accesible": accessible,
            "juegos": juegos,
        }

        if not juegos:
            payload["mensaje"] = (
                "No se encontraron juegos o el perfil de Steam es privado."
            )

        return payload

    def post(self, payload: dict[str, Any]) -> dict[str, Any]:
        steam_id = payload.get("steam_id")
        if not steam_id:
            raise ValueError("steam_id es obligatorio.")
        enrich_top = int(payload.get("enrich_top", 5))
        return self.get(steam_id, enrich_top=enrich_top)

    def _get_game_detail(self, app_id: Optional[int]) -> dict[str, Any]:
        if not app_id:
            return {}

        params = {
            "appids": app_id,
            "l": self.settings.steam_language,
            "cc": self.settings.country_code,
        }
        data = self._request_json(
            "GET",
            "https://store.steampowered.com/api/appdetails",
            params=params,
        )
        detail = data.get(str(app_id), {})
        if not detail.get("success"):
            return {}

        game_data = detail.get("data", {})
        return {
            "portada": game_data.get("header_image"),
            "descripcion": game_data.get(
                "short_description",
                "Sin descripcion.",
            ),
            "generos": [
                genre.get("description", "")
                for genre in game_data.get("genres", [])
                if genre.get("description")
            ],
            "categorias": [
                category.get("description", "")
                for category in game_data.get("categories", [])
                if category.get("description")
            ],
        }


class OpenAIManager(DataManager):
    def __init__(self, settings: APISettings, session=None) -> None:
        super().__init__(
            api_gestion="openai_recomendaciones",
            link="https://api.openai.com/v1/responses",
            key=settings.openai_api_key,
            session=session,
            timeout=settings.timeout,
        )
        self.settings = settings

    def get(
        self,
        steam_context: dict[str, Any],
        items_por_categoria: int = 3,
    ) -> dict[str, Any]:
        return self._generate_recommendations(
            steam_context,
            items_por_categoria,
        )

    def post(self, payload: dict[str, Any]) -> dict[str, Any]:
        steam_context = payload.get("steam_context")
        if not steam_context:
            raise ValueError("steam_context es obligatorio.")
        items_por_categoria = int(payload.get("items_por_categoria", 3))
        return self._generate_recommendations(
            steam_context,
            items_por_categoria,
        )

    def _generate_recommendations(
        self,
        steam_context: dict[str, Any],
        items_por_categoria: int,
    ) -> dict[str, Any]:
        self._ensure_key("OPENAI_API_KEY")

        headers = {
            "Authorization": f"Bearer {self.key}",
            "Content-Type": "application/json",
        }

        prompt = self._build_prompt(steam_context, items_por_categoria)
        schema = self._build_schema()

        payload = {
            "model": self.settings.openai_model,
            "instructions": (
                "Eres un motor de recomendaciones multimedia para una app "
                "de escritorio en Qt. Debes inferir gustos del usuario a "
                "partir de su biblioteca de Steam y devolver solo JSON."
            ),
            "input": prompt,
            "text": {
                "format": {
                    "type": "json_schema",
                    "name": "recomendaciones_usuario",
                    "schema": schema,
                    "strict": True,
                }
            },
        }

        data = self._request_json(
            "POST",
            self.link,
            payload=payload,
            headers=headers,
        )
        raw_text = self._extract_output_text(data)

        try:
            recommendations = json.loads(raw_text)
        except json.JSONDecodeError as exc:
            raise ServiceError(
                "openai_recomendaciones: el modelo no devolvio JSON valido."
            ) from exc

        return {
            "api": self.api_gestion,
            "modelo": self.settings.openai_model,
            "recomendaciones": recommendations,
        }

    def _build_prompt(
        self,
        steam_context: dict[str, Any],
        items_por_categoria: int,
    ) -> str:
        context_dump = json.dumps(
            steam_context,
            ensure_ascii=False,
            indent=2,
        )
        return (
            "Analiza la biblioteca de Steam y detecta gustos, franquicias, "
            "generos y estilo de consumo. Luego recomienda exactamente "
            f"{items_por_categoria} peliculas, {items_por_categoria} libros, "
            f"{items_por_categoria} juegos y {items_por_categoria} videos de "
            "YouTube. Cada recomendacion debe incluir: titulo, motivo y "
            "query. La query debe ser corta y util para buscar en otra API. "
            "No inventes informacion fuera del contexto dado.\n\n"
            f"Contexto Steam:\n{context_dump}"
        )

    def _build_schema(self) -> dict[str, Any]:
        item_schema = {
            "type": "object",
            "properties": {
                "titulo": {"type": "string"},
                "motivo": {"type": "string"},
                "query": {"type": "string"},
            },
            "required": ["titulo", "motivo", "query"],
            "additionalProperties": False,
        }
        return {
            "type": "object",
            "properties": {
                "perfil": {
                    "type": "object",
                    "properties": {
                        "resumen": {"type": "string"},
                        "generos_probables": {
                            "type": "array",
                            "items": {"type": "string"},
                        },
                        "franquicias_detectadas": {
                            "type": "array",
                            "items": {"type": "string"},
                        },
                    },
                    "required": [
                        "resumen",
                        "generos_probables",
                        "franquicias_detectadas",
                    ],
                    "additionalProperties": False,
                },
                "peliculas": {
                    "type": "array",
                    "items": item_schema,
                },
                "libros": {
                    "type": "array",
                    "items": item_schema,
                },
                "juegos": {
                    "type": "array",
                    "items": item_schema,
                },
                "videos_youtube": {
                    "type": "array",
                    "items": item_schema,
                },
            },
            "required": [
                "perfil",
                "peliculas",
                "libros",
                "juegos",
                "videos_youtube",
            ],
            "additionalProperties": False,
        }

    def _extract_output_text(self, response_data: dict[str, Any]) -> str:
        output_text = response_data.get("output_text")
        if isinstance(output_text, str) and output_text.strip():
            return output_text

        texts: list[str] = []
        for output_item in response_data.get("output", []):
            for content in output_item.get("content", []):
                if content.get("type") == "output_text" and content.get("text"):
                    texts.append(content["text"])

        if not texts:
            raise ServiceError(
                "openai_recomendaciones: no se encontro texto util en la respuesta."
            )

        return "".join(texts)


class LibrosManager(DataManager):
    def __init__(self, settings: APISettings, session=None) -> None:
        super().__init__(
            api_gestion="libros",
            link="https://www.googleapis.com/books/v1/volumes",
            key=settings.google_books_api_key,
            session=session,
            timeout=settings.timeout,
        )

    def get(self, query: str, max_results: int = 5) -> dict[str, Any]:
        params = {
            "q": query,
            "maxResults": max_results,
        }
        if self.key:
            params["key"] = self.key

        data = self._request_json("GET", self.link, params=params)
        items = []
        for item in data.get("items", []):
            info = item.get("volumeInfo", {})
            images = info.get("imageLinks", {})
            items.append(
                {
                    "titulo": info.get("title", "Sin titulo"),
                    "autores": info.get("authors", []),
                    "editorial": info.get("publisher"),
                    "fecha_publicacion": info.get("publishedDate"),
                    "descripcion": info.get("description"),
                    "portada": images.get("thumbnail")
                    or images.get("smallThumbnail"),
                    "info": info.get("infoLink"),
                }
            )

        return {
            "api": self.api_gestion,
            "query": query,
            "cantidad": len(items),
            "resultados": items,
        }

    def post(self, payload: dict[str, Any]) -> dict[str, Any]:
        query = payload.get("query")
        if not query:
            raise ValueError("query es obligatorio.")
        max_results = int(payload.get("max_results", 5))
        return self.get(query, max_results=max_results)


class PeliculasManager(DataManager):
    def __init__(self, settings: APISettings, session=None) -> None:
        super().__init__(
            api_gestion="peliculas",
            link="https://api.themoviedb.org/3/search/movie",
            key=settings.tmdb_api_key,
            session=session,
            timeout=settings.timeout,
        )
        self.settings = settings

    def get(self, query: str, max_results: int = 5) -> dict[str, Any]:
        self._ensure_key("TMDB_API_KEY")
        params = {
            "api_key": self.key,
            "query": query,
            "language": self.settings.language,
            "page": 1,
        }
        data = self._request_json("GET", self.link, params=params)

        items = []
        for movie in data.get("results", [])[:max_results]:
            poster_path = movie.get("poster_path")
            items.append(
                {
                    "titulo": movie.get("title", "Sin titulo"),
                    "descripcion": movie.get("overview"),
                    "fecha_estreno": movie.get("release_date"),
                    "portada": (
                        f"https://image.tmdb.org/t/p/w500{poster_path}"
                        if poster_path
                        else None
                    ),
                    "info": movie.get("id"),
                }
            )

        return {
            "api": self.api_gestion,
            "query": query,
            "cantidad": len(items),
            "resultados": items,
        }

    def post(self, payload: dict[str, Any]) -> dict[str, Any]:
        query = payload.get("query")
        if not query:
            raise ValueError("query es obligatorio.")
        max_results = int(payload.get("max_results", 5))
        return self.get(query, max_results=max_results)


class SteamJuegosManager(DataManager):
    def __init__(self, settings: APISettings, session=None) -> None:
        super().__init__(
            api_gestion="steam_juegos",
            link="https://store.steampowered.com/api/storesearch/",
            session=session,
            timeout=settings.timeout,
        )
        self.settings = settings

    def get(self, query: str, max_results: int = 5) -> dict[str, Any]:
        params = {
            "term": query,
            "l": self.settings.steam_language,
            "cc": self.settings.country_code,
            "json": 1,
        }
        data = self._request_json("GET", self.link, params=params)

        items = []
        for game in data.get("items", [])[:max_results]:
            app_id = game.get("id")
            items.append(
                {
                    "appid": app_id,
                    "titulo": game.get("name", "Sin titulo"),
                    "portada": game.get("tiny_image")
                    or (
                        f"https://cdn.cloudflare.steamstatic.com/steam/apps/"
                        f"{app_id}/header.jpg"
                    ),
                    "precio": self._format_price(game.get("price")),
                    "info": self._get_store_info(app_id),
                }
            )

        return {
            "api": self.api_gestion,
            "query": query,
            "cantidad": len(items),
            "resultados": items,
        }

    def post(self, payload: dict[str, Any]) -> dict[str, Any]:
        query = payload.get("query")
        if not query:
            raise ValueError("query es obligatorio.")
        max_results = int(payload.get("max_results", 5))
        return self.get(query, max_results=max_results)

    def _get_store_info(self, app_id: Optional[int]) -> Optional[str]:
        if not app_id:
            return None

        try:
            params = {
                "appids": app_id,
                "l": self.settings.steam_language,
                "cc": self.settings.country_code,
            }
            data = self._request_json(
                "GET",
                "https://store.steampowered.com/api/appdetails",
                params=params,
            )
            detail = data.get(str(app_id), {})
        except ServiceError:
            return None

        if not detail.get("success"):
            return None

        return detail.get("data", {}).get("short_description")

    def _format_price(self, price_data: Any) -> str:
        if not isinstance(price_data, dict):
            return "No disponible"
        final_price = price_data.get("final")
        if final_price is None:
            return "No disponible"
        return f"${final_price / 100:.2f}"


class YouTubeManager(DataManager):
    def __init__(self, settings: APISettings, session=None) -> None:
        super().__init__(
            api_gestion="youtube",
            link="https://www.googleapis.com/youtube/v3/search",
            key=settings.youtube_api_key,
            session=session,
            timeout=settings.timeout,
        )

    def get(self, query: str, max_results: int = 5) -> dict[str, Any]:
        self._ensure_key("YOUTUBE_API_KEY")
        params = {
            "part": "snippet",
            "q": query,
            "maxResults": max_results,
            "type": "video",
            "key": self.key,
        }
        data = self._request_json("GET", self.link, params=params)

        items = []
        for video in data.get("items", []):
            snippet = video.get("snippet", {})
            video_id = video.get("id", {}).get("videoId")
            thumbnails = snippet.get("thumbnails", {})
            items.append(
                {
                    "titulo": snippet.get("title", "Sin titulo"),
                    "canal": snippet.get("channelTitle"),
                    "descripcion": snippet.get("description"),
                    "portada": (
                        thumbnails.get("high", {}).get("url")
                        or thumbnails.get("medium", {}).get("url")
                        or thumbnails.get("default", {}).get("url")
                    ),
                    "info": (
                        f"https://www.youtube.com/watch?v={video_id}"
                        if video_id
                        else None
                    ),
                }
            )

        return {
            "api": self.api_gestion,
            "query": query,
            "cantidad": len(items),
            "resultados": items,
        }

    def post(self, payload: dict[str, Any]) -> dict[str, Any]:
        query = payload.get("query")
        if not query:
            raise ValueError("query es obligatorio.")
        max_results = int(payload.get("max_results", 5))
        return self.get(query, max_results=max_results)


class UsuariosDBManager(DataManager):
    def __init__(self, settings: APISettings, session=None) -> None:
        super().__init__(
            api_gestion="usuarios_db",
            link=str(settings.database_path),
            session=session,
            timeout=settings.timeout,
        )
        self.settings = settings
        self.database_path = Path(self.link)
        self.database_path.parent.mkdir(parents=True, exist_ok=True)
        self._initialize_database()

    def get(self, identifier: Optional[str] = None) -> dict[str, Any]:
        with sqlite3.connect(self.database_path) as connection:
            connection.row_factory = sqlite3.Row
            cursor = connection.cursor()

            if identifier:
                cursor.execute(
                    """
                    SELECT id, username, email, steam_id, created_at
                    FROM users
                    WHERE username = ? OR email = ?
                    """,
                    (identifier, identifier),
                )
                row = cursor.fetchone()
                return {
                    "api": self.api_gestion,
                    "usuario": dict(row) if row else None,
                }

            cursor.execute(
                """
                SELECT id, username, email, steam_id, created_at
                FROM users
                ORDER BY created_at DESC
                """
            )
            users = [dict(row) for row in cursor.fetchall()]
            return {
                "api": self.api_gestion,
                "cantidad": len(users),
                "usuarios": users,
            }

    def post(self, payload: dict[str, Any]) -> dict[str, Any]:
        action = payload.get("accion", "register").lower()
        if action == "register":
            return self._register_user(
                username=payload.get("username", ""),
                email=payload.get("email", ""),
                password=payload.get("password", ""),
                steam_id=payload.get("steam_id"),
            )
        if action == "login":
            return self._login(
                identifier=payload.get("identifier", ""),
                password=payload.get("password", ""),
            )
        raise ValueError("Accion no soportada. Usa register o login.")

    def _initialize_database(self) -> None:
        with sqlite3.connect(self.database_path) as connection:
            connection.execute(
                """
                CREATE TABLE IF NOT EXISTS users (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    username TEXT NOT NULL UNIQUE,
                    email TEXT NOT NULL UNIQUE,
                    password_hash TEXT NOT NULL,
                    salt TEXT NOT NULL,
                    steam_id TEXT,
                    created_at TEXT NOT NULL
                )
                """
            )

    def _register_user(
        self,
        username: str,
        email: str,
        password: str,
        steam_id: Optional[str] = None,
    ) -> dict[str, Any]:
        if not username or not email or not password:
            raise ValueError(
                "username, email y password son obligatorios."
            )

        salt = secrets.token_hex(16)
        password_hash = self._hash_password(password, salt)
        created_at = datetime.now(timezone.utc).isoformat()

        try:
            with sqlite3.connect(self.database_path) as connection:
                cursor = connection.cursor()
                cursor.execute(
                    """
                    INSERT INTO users (
                        username, email, password_hash, salt, steam_id, created_at
                    )
                    VALUES (?, ?, ?, ?, ?, ?)
                    """,
                    (
                        username,
                        email,
                        password_hash,
                        salt,
                        steam_id,
                        created_at,
                    ),
                )
                user_id = cursor.lastrowid
        except sqlite3.IntegrityError as exc:
            raise ValueError(
                "El usuario o el email ya existen en la base de datos."
            ) from exc

        return {
            "api": self.api_gestion,
            "registrado": True,
            "usuario": {
                "id": user_id,
                "username": username,
                "email": email,
                "steam_id": steam_id,
                "created_at": created_at,
            },
        }

    def _login(self, identifier: str, password: str) -> dict[str, Any]:
        if not identifier or not password:
            raise ValueError("identifier y password son obligatorios.")

        with sqlite3.connect(self.database_path) as connection:
            connection.row_factory = sqlite3.Row
            cursor = connection.cursor()
            cursor.execute(
                """
                SELECT id, username, email, password_hash, salt, steam_id, created_at
                FROM users
                WHERE username = ? OR email = ?
                """,
                (identifier, identifier),
            )
            row = cursor.fetchone()

        if not row:
            raise ValueError("Credenciales invalidas.")

        if self._hash_password(password, row["salt"]) != row["password_hash"]:
            raise ValueError("Credenciales invalidas.")

        return {
            "api": self.api_gestion,
            "login_ok": True,
            "usuario": {
                "id": row["id"],
                "username": row["username"],
                "email": row["email"],
                "steam_id": row["steam_id"],
                "created_at": row["created_at"],
            },
        }

    def _hash_password(self, password: str, salt: str) -> str:
        return hashlib.pbkdf2_hmac(
            "sha256",
            password.encode("utf-8"),
            salt.encode("utf-8"),
            120000,
        ).hex()


class DataCoordinator:
    def __init__(self, settings: Optional[APISettings] = None) -> None:
        self.settings = settings or APISettings.from_sources()
        self.session = requests.Session()
        self.steam_usuario = SteamUsuarioManager(self.settings, self.session)
        self.openai = OpenAIManager(self.settings, self.session)
        self.libros = LibrosManager(self.settings, self.session)
        self.peliculas = PeliculasManager(self.settings, self.session)
        self.steam_juegos = SteamJuegosManager(self.settings, self.session)
        self.youtube = YouTubeManager(self.settings, self.session)
        self.usuarios = UsuariosDBManager(self.settings, self.session)
        self.apis = {
            "steam_usuario": self.steam_usuario,
            "openai": self.openai,
            "libros": self.libros,
            "peliculas": self.peliculas,
            "steam_juegos": self.steam_juegos,
            "youtube": self.youtube,
            "usuarios": self.usuarios,
        }

    def generar_recomendaciones_desde_steam(
        self,
        steam_id: str,
        items_por_categoria: int = 3,
        enrich_top: int = 5,
    ) -> dict[str, Any]:
        biblioteca = self.steam_usuario.get(steam_id, enrich_top=enrich_top)
        if not biblioteca.get("juegos"):
            return {
                "steam_id": steam_id,
                "generado_en": datetime.now(timezone.utc).isoformat(),
                "biblioteca": biblioteca,
                "mensaje": biblioteca.get(
                    "mensaje",
                    "No fue posible generar recomendaciones sin juegos visibles.",
                ),
            }

        contexto = self._build_steam_context(biblioteca)
        recomendaciones_raw = self.openai.post(
            {
                "steam_context": contexto,
                "items_por_categoria": items_por_categoria,
            }
        )
        recomendaciones = recomendaciones_raw["recomendaciones"]

        return {
            "steam_id": steam_id,
            "generado_en": datetime.now(timezone.utc).isoformat(),
            "biblioteca": biblioteca,
            "contexto": contexto,
            "perfil": recomendaciones.get("perfil", {}),
            "peliculas": self._hydrate_category(
                recomendaciones.get("peliculas", []),
                self.peliculas,
            ),
            "libros": self._hydrate_category(
                recomendaciones.get("libros", []),
                self.libros,
            ),
            "juegos": self._hydrate_category(
                recomendaciones.get("juegos", []),
                self.steam_juegos,
            ),
            "videos_youtube": self._hydrate_category(
                recomendaciones.get("videos_youtube", []),
                self.youtube,
            ),
        }

    def _hydrate_category(
        self,
        suggestions: list[dict[str, Any]],
        manager: DataManager,
    ) -> list[dict[str, Any]]:
        enriched = []
        for suggestion in suggestions:
            query = suggestion.get("query") or suggestion.get("titulo", "")
            detail = None
            error = None
            try:
                detail_response = manager.get(query, max_results=1)
                results = detail_response.get("resultados", [])
                detail = results[0] if results else None
            except (ConfigurationError, ServiceError, ValueError) as exc:
                error = str(exc)

            titulo = None
            portada = None
            if detail:
                titulo = detail.get("titulo") or suggestion.get("titulo")
                portada = detail.get("portada")
            else:
                titulo = suggestion.get("titulo")

            enriched.append(
                {
                    "sugerencia": suggestion,
                    "detalle": detail,
                    "titulo": titulo,
                    "portada": portada,
                    "error": error,
                }
            )
        return enriched

    def _build_steam_context(self, biblioteca: dict[str, Any]) -> dict[str, Any]:
        juegos = biblioteca.get("juegos", [])
        total_horas = round(
            sum(game.get("horas_jugadas", 0) for game in juegos),
            2,
        )

        genres = Counter()
        franchises = Counter()
        for game in juegos[:10]:
            for genre in game.get("generos", []):
                genres[genre] += 1
            franchise = self._guess_franchise(game.get("nombre", ""))
            if franchise:
                franchises[franchise] += 1

        return {
            "steam_id": biblioteca.get("steam_id"),
            "cantidad_juegos": biblioteca.get("cantidad", 0),
            "horas_totales": total_horas,
            "top_juegos": [
                {
                    "nombre": game.get("nombre"),
                    "horas_jugadas": game.get("horas_jugadas"),
                    "generos": game.get("generos", []),
                    "descripcion": game.get("descripcion"),
                }
                for game in juegos[:10]
            ],
            "generos_frecuentes": [
                genre
                for genre, _ in genres.most_common(6)
            ],
            "franquicias_detectadas": [
                franchise
                for franchise, count in franchises.items()
                if count > 1
            ],
            "biblioteca_resumida": [
                game.get("nombre")
                for game in juegos[:40]
            ],
        }

    def _guess_franchise(self, game_name: str) -> str:
        separators = [":", "-", "|"]
        for separator in separators:
            if separator in game_name:
                return game_name.split(separator, 1)[0].strip()

        words = game_name.split()
        if len(words) >= 2:
            return " ".join(words[:2]).strip()
        return game_name.strip()
