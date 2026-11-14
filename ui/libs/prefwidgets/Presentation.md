# Preference presentation

Presentation is optional metadata containing standard C++ values, without Qt,
GTK or application dependencies. A backend can implement a presentation or use
its ordinary controls, keeping values and configuration bindings unchanged.

## Widget-specific controls

- `ChoicePresentation::Control`: `Default`, `Cards`, `Segmented`.
- `ValuePresentation::Control`: `Default`, `Duration` (values in seconds).
- `Presentation`: common label, hint and accent, also usable on containers.

`Choice::presentation` accepts only `ChoicePresentation`; `Value::presentation`
accepts only `ValuePresentation`. Asking a choice to use a duration editor is a
compile-time error. Each typed presentation includes common text and color fields.

Option labels must retain the number and order of the original choices.
Mismatched labels fall back to the original labels. Out-of-range choice indices
are ignored. Labels, hints, option labels and preview descriptions are translated
when read, just like widget labels.

`ValuePresentation` also offers `slider_min`, `slider_max` and `ticks` for a
duration editor. These restrict its slider; the original value bounds still
apply to typed input. `Time` already represents a duration without a control hint.

## Named control presentations

All leaf widgets can request a complete host-defined control by `name`. For
example, a choice can reuse a sort-order widget supplied by the application:

```cpp
auto order = Choice::create("Sort order")
                  ->presentation({.name = "example.sort-order"})
                  ->assign({"Name", "Size"})
                  ->connect(&order_setting);
```

The name is an opaque identifier, not a toolkit or QML path, and is not translated.
The host registers an implementation for a specific widget type. The component
owns its complete appearance, while the prefwidget model retains setting updates,
validation and conditional visibility. Built-in application pages can instantiate
the same component directly.

The ordinary presentation remains the fallback when no compatible implementation
is available. For example `.control = ChoicePresentation::Control::Cards` can be
specified along with `.name` to request cards as the fallback. GTK currently uses
its ordinary controls. See the Qt Quick backend's
[registration contract](toolkits/qtquick/README.md#control-presentations).

## Theme colors

`Presentation::Color` can reference a host theme role, for example
`.accent = {.role = "secondary"}`. The host defines the palette once; preference
definitions contain no duplicated light/dark values. The Qt Quick backend resolves
roles through `Theme.values` or its standard theme properties and follows changes
while the page is open. Optional `light`/`dark` literal colors remain fallbacks
when the renderer cannot resolve the role.

## Named preview assets

A card can reference one preview asset per option:

```cpp
auto layout = Choice::create("Timer layout")
                ->presentation({
                  .control = ChoicePresentation::Control::Cards,
                  .previews = {
                    {.asset = "example.layout.rings", .description = "Circular timers"},
                    {.asset = "example.layout.bars", .description = "Stacked bars"},
                    {.asset = "example.layout.focus", .description = "One prominent timer"},
                  },
                  .preview_height = 110,
                })
                ->assign({"Rings", "Bars", "Focus"})
                ->connect(&layout_setting);
```

An asset name is an opaque application-owned identifier. The host registers its
implementation with the renderer. The definition contains no geometry, image
format, toolkit resource paths or QML references. Asset identifiers are not
translated. Different preferences and built-in application pages can reuse the
same asset.

The Qt Quick backend supports SVG/raster images (including light/dark variants)
and QML components with initial properties. See its
[registration API](toolkits/qtquick/README.md#preview-assets).
Another backend can register images or native implementations under the same
names. Toolkit-specific component code belongs to the host, never to the
preference definition.

Missing/unregistered assets produce text-only cards. Assets may be registered,
replaced or removed while a page is open. A mismatched preview list also falls
back to text-only cards. `preview_height` defaults to 52 logical pixels.

The host implements the selected setting's behavior. A preview illustrates a
choice; it does not implement the application's actual timer rendering.

## Backends

Define metadata before building a page. The Qt Quick backend implements the
enhanced presentations. GTK retains its ordinary controls and can implement the
same metadata later without changing application definitions.

Widget generation, asset lookup and presentation dispatch belong to library
backends. The [Qt Quick backend](toolkits/qtquick/README.md) exposes a reusable
QML page; hosts supply definitions, register their assets and select pages.
