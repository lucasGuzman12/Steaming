# APIs para Qt

Esta carpeta contiene una version ordenada de las APIs de ejemplo con la estructura pedida:

- `DataManager` es una clase abstracta.
- Cada API tiene su propia subclase derivada.
- Cada subclase redefine `get()` y `post()`.
- `DataCoordinator` orquesta el flujo completo:
  Steam -> OpenAI -> APIs de detalle -> respuesta final para Qt.

## Clases principales

- `DataManager`: base abstracta con `api_gestion`, `link`, `key`, `metodos`, `get()` y `post()`.
- `SteamUsuarioManager`
- `OpenAIManager`
- `LibrosManager`
- `PeliculasManager`
- `SteamJuegosManager`
- `YouTubeManager`
- `UsuariosDBManager`
- `DataCoordinator`

## Variables soportadas

Se pueden cargar desde variables de entorno o desde `apis/keys.txt`:

- `STEAM_API_KEY`
- `TMDB_API_KEY`
- `GOOGLE_BOOKS_API_KEY`
- `YOUTUBE_API_KEY`
- `OPENAI_API_KEY`
- `OPENAI_MODEL`
- `DATABASE_PATH`
- `CONTENT_LANGUAGE`
- `STEAM_LANGUAGE`
- `COUNTRY_CODE`
- `REQUEST_TIMEOUT`

## Endpoint principal

El endpoint mas util para Qt es:

`GET /recomendaciones/steam/{steam_id}`

Ese endpoint:

1. Lee la biblioteca del usuario en Steam.
2. Construye un contexto resumido.
3. Lo envia a OpenAI para generar recomendaciones.
4. Busca el detalle en libros, peliculas, juegos y YouTube.
5. Devuelve un JSON unificado listo para consumir desde Qt.
