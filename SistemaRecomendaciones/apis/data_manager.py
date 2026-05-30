from __future__ import annotations

from abc import ABC, abstractmethod
from dataclasses import dataclass
import os
from pathlib import Path
from typing import Any, Optional

import requests


class ConfigurationError(RuntimeError):
    pass


class ServiceError(RuntimeError):
    pass


def _parse_key_value_file(file_path: Path) -> dict[str, str]:
    values: dict[str, str] = {}
    if not file_path.exists():
        return values

    for raw_line in file_path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        values[key.strip()] = value.strip()
    return values


@dataclass
class APISettings:
    steam_api_key: str
    tmdb_api_key: str
    google_books_api_key: str
    youtube_api_key: str
    openai_api_key: str
    openai_model: str
    database_path: Path
    language: str
    steam_language: str
    country_code: str
    timeout: int

    @classmethod
    def from_sources(cls) -> "APISettings":
        root = Path(__file__).resolve().parent.parent
        key_sources = [
            root / "apis" / "keys.txt",
            root / "ejemplo apis" / "keys.txt",
        ]

        file_values: dict[str, str] = {}
        for key_file in key_sources:
            file_values.update(_parse_key_value_file(key_file))

        database_default = root / "data" / "users.sqlite3"

        return cls(
            steam_api_key=os.getenv(
                "STEAM_API_KEY",
                file_values.get("STEAM_API_KEY", ""),
            ),
            tmdb_api_key=os.getenv(
                "TMDB_API_KEY",
                file_values.get("TMDB_API_KEY", ""),
            ),
            google_books_api_key=os.getenv(
                "GOOGLE_BOOKS_API_KEY",
                file_values.get("GOOGLE_BOOKS_API_KEY", ""),
            ),
            youtube_api_key=os.getenv(
                "YOUTUBE_API_KEY",
                file_values.get("YOUTUBE_API_KEY", ""),
            ),
            openai_api_key=os.getenv(
                "OPENAI_API_KEY",
                file_values.get("OPENAI_API_KEY", ""),
            ),
            openai_model=os.getenv(
                "OPENAI_MODEL",
                file_values.get("OPENAI_MODEL", "gpt-5.2"),
            ),
            database_path=Path(
                os.getenv(
                    "DATABASE_PATH",
                    file_values.get("DATABASE_PATH", str(database_default)),
                )
            ),
            language=os.getenv(
                "CONTENT_LANGUAGE",
                file_values.get("CONTENT_LANGUAGE", "es-ES"),
            ),
            steam_language=os.getenv(
                "STEAM_LANGUAGE",
                file_values.get("STEAM_LANGUAGE", "spanish"),
            ),
            country_code=os.getenv(
                "COUNTRY_CODE",
                file_values.get("COUNTRY_CODE", "ar"),
            ),
            timeout=int(
                os.getenv(
                    "REQUEST_TIMEOUT",
                    file_values.get("REQUEST_TIMEOUT", "25"),
                )
            ),
        )


class DataManager(ABC):
    def __init__(
        self,
        api_gestion: str,
        link: str,
        key: str = "",
        metodos: Optional[list[str]] = None,
        session: Optional[requests.Session] = None,
        timeout: int = 25,
    ) -> None:
        self.api_gestion = api_gestion
        self.link = link
        self.key = key
        self.metodos = metodos or ["GET", "POST"]
        self.session = session or requests.Session()
        self.timeout = timeout

    def _request_json(
        self,
        method: str,
        url: str,
        *,
        params: Optional[dict[str, Any]] = None,
        payload: Optional[dict[str, Any]] = None,
        headers: Optional[dict[str, str]] = None,
    ) -> dict[str, Any]:
        try:
            response = self.session.request(
                method=method,
                url=url,
                params=params,
                json=payload,
                headers=headers,
                timeout=self.timeout,
            )
        except requests.RequestException as exc:
            raise ServiceError(
                f"{self.api_gestion}: no se pudo conectar con el servicio."
            ) from exc

        if response.status_code >= 400:
            detail = response.text[:300]
            raise ServiceError(
                f"{self.api_gestion}: error {response.status_code} al consumir "
                f"la API. Detalle: {detail}"
            )

        try:
            return response.json()
        except ValueError as exc:
            raise ServiceError(
                f"{self.api_gestion}: la respuesta no es JSON valido."
            ) from exc

    def _ensure_key(self, key_name: str) -> None:
        if not self.key:
            raise ConfigurationError(
                f"Falta configurar {key_name} para la API {self.api_gestion}."
            )

    @abstractmethod
    def get(self, *args: Any, **kwargs: Any) -> Any:
        raise NotImplementedError

    @abstractmethod
    def post(self, *args: Any, **kwargs: Any) -> Any:
        raise NotImplementedError
