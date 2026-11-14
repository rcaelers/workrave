# Preferences registry

`PreferencesRegistry` combines plugin preference definitions for the GTK and Qt
preference dialogs. Its public interface is `IPreferencesRegistry`; toolkit
builders consume `IPreferencesRegistryInternal`.

The registry's `find` returns an insertion section whose `add` method inserts a
`Def` into an existing page/panel. The registry's own `add(panelDef)` contributes
panel content. GTK and Qt expose extension hooks for existing tabs, so plugins can
target other locations without adding platform-specific controls to each tab.
Multiple definitions at one location are combined. The applet contributes its
panel to the existing User interface page and leaves shared configuration
defaults unchanged.

The insertion section selects the target and position, independently of the
definition's own page/panel IDs. Locations use `page.panel.top` or
`page.panel.bottom`; `page.panel` defaults to the bottom. There is no placement
argument on `add`, `Def`, or `PanelDef`. For example:

```cpp
using namespace ui::prefwidgets;
auto top = std::make_shared<Def>()
           << (Frame::create(N_("Example")) << Toggle::create(N_("Example option"))->connect(&setting));
auto section = registry->find("ui.general.top");
section.add(top);
```

Malformed insertion paths are logged and return an empty section. Its `add`
method safely ignores registrations; callers can also test the section with
`if (section)`. Invalid preference navigation is logged and leaves the current
page selected.

Top extensions appear before the tab's built-in controls; bottom extensions
appear after them. Multiple plugins may contribute `PanelDef`s with the same
page/panel IDs: their content shares one tab, and removing one contribution leaves
the others intact. Content order within a shared panel or insertion position is
unspecified. If panel labels differ, the chosen label is also unspecified.

