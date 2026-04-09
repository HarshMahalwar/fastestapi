#pragma once

#include <string>

namespace fastestapi {

/**
 * Returns a self-contained HTML page that loads Swagger UI from a CDN
 * and points it at the given OpenAPI spec URL.
 */
inline std::string swaggerUiHtml(const std::string& specUrl = "/openapi.json",
                                  const std::string& title  = "FastestAPI — Docs") {
    return R"html(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>)html" + title + R"html(</title>
  <link rel="stylesheet" href="https://unpkg.com/swagger-ui-dist@5/swagger-ui.css">
  <style>
    html { box-sizing: border-box; overflow-y: scroll; }
    *, *::before, *::after { box-sizing: inherit; }
    body { margin: 0; background: #fafafa; }
    .topbar { display: none; }
  </style>
</head>
<body>
  <div id="swagger-ui"></div>
  <script src="https://unpkg.com/swagger-ui-dist@5/swagger-ui-bundle.js"></script>
  <script src="https://unpkg.com/swagger-ui-dist@5/swagger-ui-standalone-preset.js"></script>
  <script>
    SwaggerUIBundle({
      url:              ")html" + specUrl + R"html(",
      dom_id:           '#swagger-ui',
      deepLinking:      true,
      presets:          [ SwaggerUIBundle.presets.apis,
                          SwaggerUIStandalonePreset ],
      layout:           "StandaloneLayout",
      defaultModelsExpandDepth: 1,
      defaultModelExpandDepth:  1,
    });
  </script>
</body>
</html>)html";
}

} // namespace fastestapi