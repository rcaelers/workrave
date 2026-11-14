// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers

#include "ui/prefwidgets/qtquick/Builder.hh"
#include "ui/prefwidgets/Widgets.hh"
#include "config/ConfiguratorFactory.hh"
#include "config/Setting.hh"

#include <QColor>
#include <QEventLoop>
#include <QFile>
#include <QQmlComponent>
#include <QTemporaryDir>
#include <QGuiApplication>
#include <QPointer>
#include <QQuickItem>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlError>
#include <QTimer>

#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>
#include <string_view>

using namespace ui::prefwidgets;
using namespace ui::prefwidgets::qtquick;

namespace
{
  template<typename Widget, typename Metadata>
  concept CanPresent = requires(Widget &widget, Metadata metadata) { widget.presentation(metadata); };

  static_assert(CanPresent<Choice, ChoicePresentation>);
  static_assert(CanPresent<Value, ValuePresentation>);
  static_assert(!CanPresent<Choice, ValuePresentation>);
  static_assert(!CanPresent<Value, ChoicePresentation>);

  struct Checks
  {
    int failures{0};
    void require(bool condition, std::string_view message)
    {
      if (!condition)
        {
          std::cerr << message << '\n';
          ++failures;
        }
    }
  };

  void check_qml_warnings(Checks &checks, const QList<QQmlError> &errors)
  {
    for (const auto &error: errors)
      {
        std::cerr << error.toString().toStdString() << '\n';
      }
    checks.require(errors.isEmpty(), "Library QML must not emit runtime warnings");
  }

  void settle()
  {
    QEventLoop loop;
    QTimer::singleShot(50, &loop, &QEventLoop::quit);
    loop.exec();
  }

  QQuickItem *find_control(QQuickItem *item, const QByteArray &type)
  {
    if (item->isVisible() && QByteArray(item->metaObject()->className()).startsWith(type))
      {
        return item;
      }
    for (auto *child: item->childItems())
      {
        if (auto *found = find_control(child, type); found != nullptr)
          {
            return found;
          }
      }
    return nullptr;
  }
  QQuickItem *find_named(QQuickItem *item, const QString &name)
  {
    if (item->objectName() == name)
      {
        return item;
      }
    for (auto *child: item->childItems())
      {
        if (auto *found = find_named(child, name); found != nullptr)
          {
            return found;
          }
      }
    return nullptr;
  }

  void check_control_presentations(Checks &checks)
  {
    auto config = workrave::config::ConfiguratorFactory::create(workrave::config::ConfigFileFormat::Ini);
    workrave::config::Setting<int> order(config, "example/order", 0);
    workrave::config::Setting<bool> enabled(config, "example/enabled", true);
    auto choice = Choice::create("Sort order")
                    ->presentation({.control = ChoicePresentation::Control::Cards, .name = "example.sort"})
                    ->assign({"Name", "Size"})
                    ->connect(&order)
                    ->when(&enabled);
    auto definition = std::make_shared<Def>() << choice;
    std::unique_ptr<PageModel> page(Builder::build(definition));
    QTemporaryDir artwork;
    QFile file(artwork.filePath("SortControl.qml"));
    checks.require(artwork.isValid() && file.open(QIODevice::WriteOnly), "Custom control fixture must be available");
    if (!file.isOpen())
      {
        return;
      }
    file.write(R"(
import QtQuick
Item {
    id: root
    objectName: "customChoice"
    required property var preference
    implicitHeight: 73
    readonly property int selection: preference ? preference.currentIndex : -1
    signal selected(int index)
    onSelected: (index) => { if (preference) preference.setCurrentIndex(index) }
}
)");
    file.close();
    QQuickView view;
    QObject::connect(view.engine(), &QQmlEngine::warnings, &view, [&checks](const QList<QQmlError> &errors) {
      check_qml_warnings(checks, errors);
    });
    QQmlComponent registration(view.engine());
    registration.setData(R"(
import QtQuick
import PrefWidgets 1.0
QtObject {
    function registerPresentation(type, source) {
        PresentationRegistry.registerComponent("example.sort", type, source)
    }
    function removePresentation() { PresentationRegistry.unregister("example.sort") }
}
)",
                         QUrl("file:///PrefwidgetControlRegistration.qml"));
    std::unique_ptr<QObject> host(registration.create());
    checks.require(host != nullptr, "The host must be able to register complete controls");
    if (host == nullptr)
      {
        return;
      }
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/qt/qml/PrefWidgets/Page.qml"));
    checks.require(view.rootObject() != nullptr, "The control presentation page must load");
    if (view.rootObject() == nullptr)
      {
        return;
      }
    view.rootObject()->setProperty("model", QVariant::fromValue(static_cast<QObject *>(page.get())));
    view.resize(600, 300);
    view.show();
    settle();
    checks.require(find_control(view.rootObject(), "PrefCardsRow") != nullptr,
                   "An unregistered control presentation must use the ordinary renderer");
    const auto register_presentation = [&](const QString &type) {
      checks.require(QMetaObject::invokeMethod(host.get(),
                                               "registerPresentation",
                                               Q_ARG(QVariant, type),
                                               Q_ARG(QVariant, QUrl::fromLocalFile(file.fileName()))),
                     "Control registration must be callable by the host");
      settle();
    };
    register_presentation("choice");
    auto *control = find_named(view.rootObject(), "customChoice");
    checks.require(control != nullptr && control->width() > 0 && control->height() == 73,
                   "A registered control must receive its live model and determine row height");
    checks.require(find_control(view.rootObject(), "PrefCardsRow") == nullptr,
                   "The complete custom control must replace the built-in renderer");
    if (control != nullptr)
      {
        QMetaObject::invokeMethod(control, "selected", Q_ARG(int, 1));
        checks.require(order() == 1, "Custom control edits must save through the preference model");
        order.set(0);
        checks.require(control->property("selection") == 0, "External setting changes must reach custom controls");
        enabled.set(false);
        settle();
        checks.require(!control->isVisible(), "Custom controls must retain conditional visibility");
        enabled.set(true);
      }
    register_presentation("value");
    checks.require(find_named(view.rootObject(), "customChoice") == nullptr
                     && find_control(view.rootObject(), "PrefCardsRow") != nullptr,
                   "A presentation registered for a different widget type must fall back safely");
    register_presentation("choice");
    checks.require(find_named(view.rootObject(), "customChoice") != nullptr,
                   "Replacing a registration must update the rendered control");
    QMetaObject::invokeMethod(host.get(), "removePresentation");
    settle();
    checks.require(find_named(view.rootObject(), "customChoice") == nullptr
                     && find_control(view.rootObject(), "PrefCardsRow") != nullptr,
                   "Removing a control presentation must restore the ordinary renderer");
  }

} // namespace

int
main(int argc, char **argv)
try
  {
    QGuiApplication application(argc, argv);
    Checks checks;
    check_control_presentations(checks);
    auto config = workrave::config::ConfiguratorFactory::create(workrave::config::ConfigFileFormat::Ini);
    workrave::config::Setting<int> mode(config, "example/mode", 0);
    workrave::config::Setting<int> level(config, "example/level", 0);
    workrave::config::Setting<int> seconds(config, "example/seconds", 12);
    workrave::config::Setting<bool> enabled(config, "example/enabled", true);
    workrave::config::Setting<std::string> name(config, "example/name", std::string{"Example"});

    auto cards = Choice::create("Storage")
                   ->presentation({
                     .control = ChoicePresentation::Control::Cards,
                     .previews =
                       {
                         {.asset = "example.local", .description = "Local storage diagram"},
                         {.asset = "example.remote", .description = "Remote storage diagram"},
                       },
                     .preview_height = 80,
                   })
                   ->assign({"Local", "Remote"})
                   ->connect(&mode)
                   ->when(&enabled);
    auto segmented = Choice::create("Level")
                       ->presentation({.control = ChoicePresentation::Control::Segmented, .accent = {.role = "secondary"}})
                       ->assign({"A", "B", "C", "D", "E"})
                       ->connect(&level);
    auto duration = Value::create("Delay", 1, 999)
                      ->presentation({
                        .control = ValuePresentation::Control::Duration,
                        .slider_min = 2,
                        .slider_max = 30,
                        .ticks = {2, 10, 30},
                      })
                      ->connect(&seconds);
    auto toggle = Toggle::create("Enabled")->connect(&enabled);
    auto entry = Entry::create("Name")->connect(&name);
    auto definition = std::make_shared<Def>()
                      << (Box::create() << (Frame::create("Settings")
                                            << (Box::create() << cards << segmented << duration << toggle << entry)));

    std::unique_ptr<PageModel> page(Builder::build(definition));
    checks.require(page != nullptr && page->groups().size() == 1, "Builder must render a standalone definition");
    if (page == nullptr || page->groups().isEmpty())
      {
        return 1;
      }
    auto *group = page->groups().front().value<GroupModel *>();
    checks.require(group->rows().size() == 5, "Nested boxes must retain every leaf");
    if (group->rows().size() != 5)
      {
        return 1;
      }
    auto *card_row = group->rows().at(0).value<RowModel *>();
    auto *segmented_row = group->rows().at(1).value<RowModel *>();
    auto *duration_row = group->rows().at(2).value<RowModel *>();
    checks.require(card_row->presentation() == "cards" && card_row->options().size() == 2,
                   "Cards must support application-independent option counts");
    checks.require(segmented_row->presentation() == "segmented" && segmented_row->options().size() == 5,
                   "Segments must support application-independent option counts");
    const auto previews = card_row->previews();
    checks.require(previews.size() == 2 && previews.at(0).toMap().value("asset") == "example.local"
                     && previews.at(1).toMap().value("description") == "Remote storage diagram"
                     && card_row->previewHeight() == 80,
                   "Preview references must preserve asset names, descriptions and requested height");
    card_row->setCurrentIndex(1);
    checks.require(mode() == 1, "Card choices must save settings");
    card_row->setCurrentIndex(9);
    checks.require(mode() == 1, "Invalid choice indices must be ignored");
    duration_row->setTimeNorm(1.0);
    checks.require(seconds() == 30, "Duration sliders must use presentation bounds");
    duration_row->setSeconds(800);
    checks.require(seconds() == 800, "Typed durations must retain native widget bounds");
    seconds.set(12);
    checks.require(duration_row->timeDisplay() == "0:12", "External changes must update models");

    ActivePageModel active;
    active.set_page(page.get());
    QQuickView view;
    QObject::connect(view.engine(), &QQmlEngine::warnings, &view, [&checks](const QList<QQmlError> &errors) {
      check_qml_warnings(checks, errors);
    });
    QTemporaryDir artwork;
    checks.require(artwork.isValid(), "Preview artwork directory must be available");
    const auto write_artwork = [&checks, &artwork](const QString &name, const QByteArray &content) {
      const auto path = artwork.filePath(name);
      QFile file(path);
      checks.require(file.open(QIODevice::WriteOnly) && file.write(content) == content.size(), "Preview artwork must be saved");
      return QUrl::fromLocalFile(path);
    };
    const auto image = write_artwork(
      "light.svg",
      R"(<svg xmlns="http://www.w3.org/2000/svg" width="60" height="20"><rect width="60" height="20" fill="#456789"/></svg>)");
    const auto dark_image = write_artwork(
      "dark.svg",
      R"(<svg xmlns="http://www.w3.org/2000/svg" width="60" height="20"><rect width="60" height="20" fill="#98abcd"/></svg>)");
    const auto component = write_artwork("Diagram.qml", R"(
import QtQuick
Rectangle {
    objectName: "examplePreview"
    required property int columns
    color: "#567890"
}
)");
    QQmlComponent registration(view.engine());
    registration.setData(R"(
import QtQuick
import PrefWidgets 1.0
QtObject {
    function registerAssets(image, darkImage, component) {
        PreviewRegistry.registerImage("example.local", image, darkImage)
        PreviewRegistry.registerComponent("example.remote", component, { columns: 4 })
    }
    function replacePreview(component) {
        PreviewRegistry.registerComponent("example.remote", component, { columns: 7 })
    }
    function removeAssets() {
        PreviewRegistry.unregister("example.local")
        PreviewRegistry.unregister("example.remote")
    }
    function useDarkTheme() {
        Theme.colorScheme = 2
        Theme.values = { secondary: "#987654" }
    }
}
)",
                         QUrl("file:///PrefwidgetPreviewRegistration.qml"));
    std::unique_ptr<QObject> assets(registration.create());
    checks.require(assets != nullptr, "The host must be able to register previews without application resources");
    if (assets == nullptr)
      {
        return 1;
      }
    const auto register_assets = [&] {
      checks.require(QMetaObject::invokeMethod(assets.get(),
                                               "registerAssets",
                                               Q_ARG(QVariant, image),
                                               Q_ARG(QVariant, dark_image),
                                               Q_ARG(QVariant, component)),
                     "Asset registration must be callable by the host");
    };
    register_assets();
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/qt/qml/PrefWidgets/Page.qml"));
    checks.require(view.status() != QQuickView::Error && view.rootObject() != nullptr,
                   "The library QML module must load without application resources");
    if (view.rootObject() == nullptr)
      {
        return 1;
      }
    view.rootObject()->setProperty("model", QVariant::fromValue(static_cast<QObject *>(&active)));
    view.resize(700, 800);
    view.show();
    settle();
    auto *card_control = find_control(view.rootObject(), "PrefCardsRow");
    auto *segment_control = find_control(view.rootObject(), "PrefSegmentedRow");
    auto *duration_control = find_control(view.rootObject(), "PrefTimeControl");
    checks.require(card_control != nullptr && segment_control != nullptr && duration_control != nullptr,
                   "Library page must select all requested presentation renderers");
    auto *custom_preview = find_named(view.rootObject(), "examplePreview");
    checks.require(custom_preview != nullptr && custom_preview->property("columns") == 4,
                   "Registered QML previews must receive initial required properties");
    QMetaObject::invokeMethod(assets.get(), "replacePreview", Q_ARG(QVariant, component));
    settle();
    custom_preview = find_named(view.rootObject(), "examplePreview");
    checks.require(custom_preview != nullptr && custom_preview->property("columns") == 7,
                   "Replacing a component registration must refresh its initial properties");
    auto *preview_image = find_control(view.rootObject(), "QQuickImage");
    checks.require(preview_image != nullptr && preview_image->property("source").toUrl() == image
                     && preview_image->property("status").toInt() == 1,
                   "Registered SVG previews must load successfully");
    QMetaObject::invokeMethod(assets.get(), "useDarkTheme");
    settle();

    checks.require(preview_image != nullptr && preview_image->property("source").toUrl() == dark_image,
                   "Image previews must follow theme changes");
    QMetaObject::invokeMethod(assets.get(), "removeAssets");
    settle();
    checks.require(find_named(view.rootObject(), "examplePreview") == nullptr
                     && find_control(view.rootObject(), "QQuickImage") == nullptr,
                   "Unknown or removed assets must fall back to text cards");
    register_assets();
    settle();
    checks.require(find_named(view.rootObject(), "examplePreview") != nullptr, "Late registration must refresh existing cards");
    if (card_control != nullptr)
      {
        QMetaObject::invokeMethod(card_control, "selected", Q_ARG(int, 0));
        checks.require(mode() == 0, "QML card selection must reach the setting");
      }
    if (segment_control != nullptr)
      {
        checks.require(segment_control->property("accentColor").value<QColor>() == QColor("#987654"),
                       "Named accents must resolve from the shared theme and update when it changes");
        QMetaObject::invokeMethod(segment_control, "selected", Q_ARG(int, 4));
        checks.require(level() == 4, "QML segmented selection must reach the setting");
      }
    if (duration_control != nullptr)
      {
        QMetaObject::invokeMethod(duration_control, "committed", Q_ARG(int, 19));
        checks.require(seconds() == 19, "QML duration editing must reach the setting");
      }
    view.rootObject()->setProperty("model", QVariant::fromValue(static_cast<QObject *>(page.get())));
    settle();
    checks.require(find_control(view.rootObject(), "PrefCardsRow") != nullptr,
                   "PageModel must be directly renderable without a navigation adapter");
    auto extra = std::make_shared<Def>() << (Frame::create("Additional") << Entry::create("Extra")->connect(&name));
    Builder::append(extra, page.get());
    settle();
    checks.require(page->groups().size() == 2 && active.groups().size() == 2,
                   "Appending definitions must update page and active models");
    view.rootObject()->setProperty("model", QVariant::fromValue(static_cast<QObject *>(&active)));
    settle();
    enabled.set(false);
    settle();
    checks.require(!card_row->enabled() && find_control(view.rootObject(), "PrefCardsRow") == nullptr,
                   "Conditional settings must update rendered controls");
    enabled.set(true);
    settle();

    // Definitions may outlive the generated UI. Late updates must be harmless.
    auto temporary = Choice::create("Temporary")->assign({"A", "B"})->connect(&mode);
    auto temporary_definition = std::make_shared<Def>() << temporary;
    std::unique_ptr<PageModel> temporary_page(Builder::build(temporary_definition));
    QPointer<RowModel> old_row = temporary_page->groups().front().value<GroupModel *>()->rows().front().value<RowModel *>();
    temporary_page.reset();
    checks.require(old_row.isNull(), "The page must own its generated UI models");
    mode.set(1);

    checks.require(Builder::build(nullptr) == nullptr, "Empty definitions must fail safely");
    Builder::append(nullptr, page.get());
    std::cout << "Standalone prefwidgets Qt Quick checks: " << checks.failures << " failures\n";
    return checks.failures == 0 ? 0 : 1;
  }
catch (const std::exception &error)
  {
    std::fputs(error.what(), stderr);
    std::fputs("\n", stderr);
    return 1;
  }
