# spider-bars

Barra de ventanas de SpiderOS: fork de `hyprbars` (hyprwm/hyprland-plugins, commit 7644cec para Hyprland 0.56.x) con un estilo plano:

`( ○ ) Título ....................  [ bloque ][ + − × ]`

Opciones nuevas (dentro de `plugin.hyprbars`):

| opción | tipo | para qué |
|---|---|---|
| `spider_style` | bool | activa el estilo plano |
| `col.buttons_box` | color | contenedor de los botones |
| `col.accent_box` | color | bloque a la izquierda de los botones |
| `col.icon` | color | aro antes del título |
| `accent_box_width` | int | ancho del bloque |

Instalar:

```bash
hyprpm add https://github.com/xFractu/spider-bars
hyprpm enable hyprbars
```
