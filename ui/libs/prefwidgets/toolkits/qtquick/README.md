# Qt Quick preference renderer

This backend builds page models and renders prefwidget definitions in QML.
Link `workrave-libs-ui-prefwidgets-qtquick`; no application toolkit code or
application QML resources are needed.

The sibling `qt` backend generates Qt Widgets controls. The `qtquick` backend
generates Qt Quick controls. Both consume the same toolkit-neutral definitions;
`gtkmm` is the GTK renderer.

## Using a page

```cpp
using namespace ui::prefwidgets::qtquick;

Builder::initialize();
auto *page = Builder::build(definition, owner);
```

Set that page as the `model` property of the exported QML component:

```qml
import QtQuick
import PrefWidgets 1.0

Page {
    model: preferenceModel
}
```

The library registers its resources in `Builder::initialize()`, and
`Builder::build()` calls it automatically. Call `initialize()` before loading
standalone controls without a model. The module is embedded in the library
under `qrc:/qt/qml/PrefWidgets`.

`PageModel` exposes groups, and `GroupModel` exposes rows. These are generated
by the library: hosts should not construct individual controls or inspect widget
types. `Builder::append()` adds another definition to an existing page and
updates rendered content.

For navigation, `ActivePageModel` can select one page or combine two pages.
Hosts decide which definitions belong to a tab; the library owns widget-tree
traversal, model creation, setting callbacks and presentation dispatch.
Generated models are QObject children of their page. Definition callbacks use
guarded pointers, so destroying a page before its definition is safe.

## Styling

`Theme` is a per-engine singleton with native palette defaults. Hosts may set
`Theme.values` to a map of theme roles, such as `background`, `surface`, `text`,
`textMuted`, `accent`, `accentSoft`, `border` and `borderSubtle`. Typography and
spacing roles can also be overridden. `Theme.colorScheme` follows the platform
by default and can be explicitly set by the host.

Application-specific palettes belong to the host. The library does not import
or call an application's theme component.

## Control presentations

`PresentationRegistry` registers complete controls by name and widget type.
This is separate from registering an illustration inside a card:

```qml
PresentationRegistry.registerComponent("example.sort-order", "choice",
    Qt.resolvedUrl("SortOrder.qml"))
```

A definition requests this with `presentation({.name = "example.sort-order"})`.
Supported type names are `toggle`, `time`, `value`, `choice` and `entry`. The
library loads only implementations registered for the actual widget type.
Unknown names, incompatible types and components that fail to load fall back to
the ordinary renderer. Registration is per engine; registering the same name
replaces it, and `unregister(name)` restores the fallback. Repeating an identical
registration does not reload the component.

The component must be an `Item` with an `implicitHeight` and a `preference`
property. The library supplies this property during creation, so it can be
required. The component receives the live `RowModel`, including `label`, `hint`,
`options` and its type's value/edit methods. A minimal choice control could be:

```qml
import QtQuick
import PrefWidgets 1.0

PrefChoiceRow {
    required property var preference
    label: preference.label
    options: preference.options
    currentIndex: preference.currentIndex
    onSelected: (index) => preference.setCurrentIndex(index)
}
```

The library supplies the available width, uses the component's `implicitHeight`
and retains conditional row visibility. The component should bind to model
properties rather than copying their initial values. Edits go through the model:
`setCurrentIndex` for choices, `setChecked` for toggles, `setSeconds` or
`increment`/`decrement` for values and time, and `setEntryText` for text entries.
It may supply its own labels, options, previews and arrangement, as long as choice
indices keep their defined meaning.

A component can instead make `preference` optional and expose its usual public
properties/signals. This lets a built-in page instantiate the same QML directly
and bind to its own application bridge. The same component can then be registered
with prefwidgets for reuse elsewhere. Adding a named presentation requires no application widget-dispatch or library changes.

## Preview assets

`PreviewRegistry` is a per-QML-engine registry. Register application assets
before displaying preferences, or later; existing cards update automatically.
Use names unique to the host/plugin. Registering the same name replaces its
previous implementation. `unregister(name)` removes it. Unknown names safely
produce text-only cards.

```qml
import QtQuick
import PrefWidgets 1.0

QtObject {
    Component.onCompleted: {
        PreviewRegistry.registerImage("example.layout.rings",
            Qt.resolvedUrl("rings.svg"), Qt.resolvedUrl("rings-dark.svg"))
        PreviewRegistry.registerComponent("example.layout.bars",
            Qt.resolvedUrl("LayoutPreview.qml"), { style: "bars", count: 3 })
    }
}
```

The third `registerImage` argument is optional; without it, both themes use the
same image. Image formats supported by the installed Qt image plugins are
available, including SVG. The source URLs should be absolute, for example from
`Qt.resolvedUrl`, or data URIs. QML component properties are passed during
creation, so required properties are supported. The component fills the preview
area and may use its own application theme. Registration stores URLs and data;
it does not retain the lifetime of the registering component.

`Preview { asset: "example.layout.bars" }` renders the same asset outside a
preference card, allowing built-in pages to share the exact implementation.
`Preview.available` indicates whether the name is registered. The library
contains no application-specific shapes, counts, colors or layout names.

The registry is deliberately part of this backend: QML components and Qt resource
URLs must not enter toolkit-independent definitions. A GTK backend can map the
same asset names to images or native drawing code.

## Adding a presentation

The generic contract is in [Presentation.hh](../../include/ui/prefwidgets/Presentation.hh).
Add the renderer and its control/presentation mapping here in the library:

- `src/Models.cc` maps widget-specific definitions to presentation names and exposes
  their data.
- `qml/PrefWidgets/Page.qml` dispatches rows to controls.
- `qml/PrefWidgets/` contains the reusable controls and theme.
- `qml/PrefWidgets/qmldir` and `prefwidgets_quick.qrc` export additional controls.

Applications using `Page` require no renderer or toolkit changes. GTK can
implement the same presentation in its own library backend while retaining its
ordinary controls as a fallback.

## Verification

With `WITH_TESTS=ON`, `prefwidgets-qtquick-test` builds a standalone consumer.
It uses generic settings and definitions, loads only the library QML module,
and checks nested containers, presentation selection, setting updates, late
appends, direct page use, page/definition lifetime, SVG light/dark assets, QML
required properties, live asset replacement/removal, complete control registration,
setting updates and safe type/registration fallback. It runs offscreen through
CTest as `prefwidgets-qtquick`.
