from __future__ import annotations

from typing import Any, Optional

from fastapi import FastAPI, HTTPException, Query
from pydantic import BaseModel, Field

from .data_manager import ConfigurationError, ServiceError
from .services import DataCoordinator


app = FastAPI(
    title="APIs de Recomendacion para Qt",
    version="1.0.0",
    description=(
        "Capa intermedia entre el programa Qt y las APIs de Steam, "
        "OpenAI, libros, peliculas, juegos, YouTube y usuarios."
    ),
)

coordinator = DataCoordinator()


class BusquedaPayload(BaseModel):
    query: str = Field(..., min_length=1)
    max_results: int = Field(default=5, ge=1, le=10)


class SteamPayload(BaseModel):
    steam_id: str = Field(..., min_length=1)
    enrich_top: int = Field(default=5, ge=0, le=10)


class OpenAIPayload(BaseModel):
    steam_context: dict[str, Any]
    items_por_categoria: int = Field(default=5, ge=1, le=50)


class RegisterPayload(BaseModel):
    username: str = Field(..., min_length=3)
    email: str = Field(..., min_length=5)
    password: str = Field(..., min_length=4)
    steam_id: Optional[str] = None


class LoginPayload(BaseModel):
    identifier: str = Field(..., min_length=1)
    password: str = Field(..., min_length=4)


def _model_to_dict(model: BaseModel) -> dict[str, Any]:
    if hasattr(model, "model_dump"):
        return model.model_dump()
    return model.dict()


def _raise_http(exc: Exception) -> None:
    if isinstance(exc, ConfigurationError):
        raise HTTPException(status_code=500, detail=str(exc))
    if isinstance(exc, ServiceError):
        raise HTTPException(status_code=502, detail=str(exc))
    if isinstance(exc, ValueError):
        raise HTTPException(status_code=400, detail=str(exc))
    raise HTTPException(status_code=500, detail="Error interno no controlado.")


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok"}


@app.get("/apis")
def listar_apis() -> dict[str, list[str]]:
    return {"apis": list(coordinator.apis.keys())}


@app.get("/steam/usuario/{steam_id}")
def steam_usuario_get(
    steam_id: str,
    enrich_top: int = Query(default=5, ge=0, le=10),
) -> dict[str, Any]:
    try:
        return coordinator.steam_usuario.get(steam_id, enrich_top=enrich_top)
    except Exception as exc:
        _raise_http(exc)


@app.post("/steam/usuario")
def steam_usuario_post(payload: SteamPayload) -> dict[str, Any]:
    try:
        return coordinator.steam_usuario.post(_model_to_dict(payload))
    except Exception as exc:
        _raise_http(exc)


@app.get("/libros")
def libros_get(
    query: str,
    max_results: int = Query(default=5, ge=1, le=10),
) -> dict[str, Any]:
    try:
        return coordinator.libros.get(query, max_results=max_results)
    except Exception as exc:
        _raise_http(exc)


@app.post("/libros")
def libros_post(payload: BusquedaPayload) -> dict[str, Any]:
    try:
        return coordinator.libros.post(_model_to_dict(payload))
    except Exception as exc:
        _raise_http(exc)


@app.get("/peliculas")
def peliculas_get(
    query: str,
    max_results: int = Query(default=5, ge=1, le=10),
) -> dict[str, Any]:
    try:
        return coordinator.peliculas.get(query, max_results=max_results)
    except Exception as exc:
        _raise_http(exc)


@app.post("/peliculas")
def peliculas_post(payload: BusquedaPayload) -> dict[str, Any]:
    try:
        return coordinator.peliculas.post(_model_to_dict(payload))
    except Exception as exc:
        _raise_http(exc)


@app.get("/steam/juegos")
def steam_juegos_get(
    query: str,
    max_results: int = Query(default=5, ge=1, le=10),
) -> dict[str, Any]:
    try:
        return coordinator.steam_juegos.get(query, max_results=max_results)
    except Exception as exc:
        _raise_http(exc)


@app.post("/steam/juegos")
def steam_juegos_post(payload: BusquedaPayload) -> dict[str, Any]:
    try:
        return coordinator.steam_juegos.post(_model_to_dict(payload))
    except Exception as exc:
        _raise_http(exc)


@app.get("/youtube")
def youtube_get(
    query: str,
    max_results: int = Query(default=5, ge=1, le=10),
) -> dict[str, Any]:
    try:
        return coordinator.youtube.get(query, max_results=max_results)
    except Exception as exc:
        _raise_http(exc)


@app.post("/youtube")
def youtube_post(payload: BusquedaPayload) -> dict[str, Any]:
    try:
        return coordinator.youtube.post(_model_to_dict(payload))
    except Exception as exc:
        _raise_http(exc)


@app.post("/openai/recomendaciones")
def openai_post(payload: OpenAIPayload) -> dict[str, Any]:
    try:
        return coordinator.openai.post(_model_to_dict(payload))
    except Exception as exc:
        _raise_http(exc)


@app.get("/recomendaciones/steam/{steam_id}")
def recomendaciones_desde_steam(
    steam_id: str,
    items_por_categoria: int = Query(default=5, ge=1, le=5),
    enrich_top: int = Query(default=5, ge=0, le=10),
) -> dict[str, Any]:
    try:
        return coordinator.generar_recomendaciones_desde_steam(
            steam_id,
            items_por_categoria=items_por_categoria,
            enrich_top=enrich_top,
        )
    except Exception as exc:
        _raise_http(exc)


@app.get("/usuarios")
def usuarios_get(identifier: Optional[str] = None) -> dict[str, Any]:
    try:
        return coordinator.usuarios.get(identifier)
    except Exception as exc:
        _raise_http(exc)


@app.post("/usuarios/registro")
def usuarios_register(payload: RegisterPayload) -> dict[str, Any]:
    try:
        body = _model_to_dict(payload)
        body["accion"] = "register"
        return coordinator.usuarios.post(body)
    except Exception as exc:
        _raise_http(exc)


@app.post("/usuarios/login")
def usuarios_login(payload: LoginPayload) -> dict[str, Any]:
    try:
        body = _model_to_dict(payload)
        body["accion"] = "login"
        return coordinator.usuarios.post(body)
    except Exception as exc:
        _raise_http(exc)
