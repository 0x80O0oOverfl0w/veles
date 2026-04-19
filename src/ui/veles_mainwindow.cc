/*
 * Copyright 2017 CodiLime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "ui/veles_mainwindow.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QUrl>

#include "client/localdbifwrapper.h"
#include "dbif/method.h"
#include "dbif/promise.h"
#include "dbif/types.h"
#include "dbif/universe.h"
#include "ui/databaseinfo.h"
#include "ui/dialogs/optionsdialog.h"
#include "ui/hexeditwidget.h"
#include "ui/logwidget.h"

#include "util/settings/shortcuts.h"
#include "util/version.h"

namespace veles {
namespace ui {

using util::settings::shortcuts::ShortcutsModel;

/*****************************************************************************/
/* VelesMainWindow - Public methods */
/*****************************************************************************/

VelesMainWindow::VelesMainWindow()
    : log_dock_widget_(nullptr) {
  setAcceptDrops(true);
  resize(1024, 768);
  init();
}

void VelesMainWindow::addFile(const QString& path) {
  connection_manager_->openLocalFile(path);
}

/*****************************************************************************/
/* VelesMainWindow - Protected methods */
/*****************************************************************************/

void VelesMainWindow::dropEvent(QDropEvent* ev) {
  QList<QUrl> urls = ev->mimeData()->urls();
  for (const auto& url : urls) {
    if (!url.isLocalFile()) {
      continue;
    }
    createFileBlob(url.toLocalFile());
  }
}

void VelesMainWindow::dragEnterEvent(QDragEnterEvent* ev) { ev->accept(); }

void VelesMainWindow::showEvent(QShowEvent* /*event*/) { emit shown(); }

/*****************************************************************************/
/* VelesMainWindow - Private Slots */
/*****************************************************************************/

void VelesMainWindow::open() {
  connection_manager_->openLocalFileDialog();
}

void VelesMainWindow::newFile() { createFileBlob(""); }

void VelesMainWindow::openLocalFile() {
  connection_manager_->openLocalFileDialog();
}

void VelesMainWindow::about() {
  QMessageBox::about(this, tr("About Veles"),
                     tr("This is Veles, a binary analysis tool and editor.\n"
                        "Version: %1\n"
                        "\n"
                        "Report bugs to contact@veles.io\n"
                        "IRC: #veles@freenode.net\n"
                        "https://veles.io/\n"
                        "\n"
                        "Copyright 2017 CodiLime\n"
                        "Licensed under the Apache License, Version 2.0\n")
                         .arg(util::version::string));
}

/*****************************************************************************/
/* VelesMainWindow - Private methods */
/*****************************************************************************/

void VelesMainWindow::init() {
  connection_manager_ = new ConnectionManager(this);
  setConnectionManager(connection_manager_);
  createActions();
  createMenus();
  createLogWindow();

  connect(connection_manager_, &ConnectionManager::localFileOpened,
          this, &VelesMainWindow::createDb);
  connect(connection_manager_, &ConnectionManager::localFileOpenFailed,
          [this](const QString& error) {
            QMessageBox::warning(
                nullptr, tr("File Open Error"),
                QString(tr("Failed to open file: %1")).arg(error));
          });

  options_dialog_ = new OptionsDialog(this);

  bringDockWidgetToFront(log_dock_widget_);
  log_dock_widget_->setFocus(Qt::OtherFocusReason);

  shortcuts_dialog_ = new ShortcutsDialog(this);
}

void VelesMainWindow::createActions() {
  //  new_file_act_ =
  //  ShortcutsModel::getShortcutsModel()->createQAction(util::settings::shortcuts::NEW_FILE,
  //  this, Qt::ApplicationShortcut); new_file_act_->setStatusTip(tr("Open a new
  //  file")); connect(new_file_act_, &QAction::triggered, this,
  //  &VelesMainWindow::newFile);

  open_act_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::OPEN_FILE, this, QIcon(":/images/open.png"),
      Qt::ApplicationShortcut);
  open_act_->setStatusTip(tr("Open an existing file"));
  connect(open_act_, &QAction::triggered, this, &VelesMainWindow::open);

  exit_act_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::EXIT_APPLICATION, this,
      Qt::ApplicationShortcut);
  exit_act_->setStatusTip(tr("Exit the application"));
  connect(exit_act_, &QAction::triggered, qApp, &QApplication::closeAllWindows);

  show_log_act_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::SHOW_LOG, this, Qt::ApplicationShortcut);
  show_log_act_->setStatusTip(tr("Show log"));
  connect(show_log_act_, &QAction::triggered, this, &VelesMainWindow::showLog);

  about_act_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::SHOW_ABOUT, this, Qt::ApplicationShortcut);
  about_act_->setStatusTip(tr("Show the application's About box"));
  connect(about_act_, &QAction::triggered, this, &VelesMainWindow::about);

  about_qt_act_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::SHOW_ABOUT_QT, this, Qt::ApplicationShortcut);
  about_qt_act_->setStatusTip(tr("Show the Qt library's About box"));
  connect(about_qt_act_, &QAction::triggered, qApp, &QApplication::aboutQt);

  options_act_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::SHOW_OPTIONS, this, Qt::ApplicationShortcut);
  options_act_->setStatusTip(
      tr("Show the dialog to select applications options"));
  connect(options_act_, &QAction::triggered, this,
          [this]() { options_dialog_->show(); });

  shortcuts_act_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::SHOW_SHORTCUT_OPTIONS, this,
      Qt::ApplicationShortcut);
  shortcuts_act_->setStatusTip(
      tr("Show the dialog to customize keyboard shortcuts"));
  connect(shortcuts_act_, &QAction::triggered, this,
          [this]() { shortcuts_dialog_->exec(); });
}

void VelesMainWindow::createMenus() {
  file_menu_ = menuBar()->addMenu(tr("&File"));

  // Not implemented yet.
  // fileMenu->addAction(new_file_act_);

  file_menu_->addAction(open_act_);
  file_menu_->addSeparator();
  file_menu_->addAction(options_act_);
  file_menu_->addAction(shortcuts_act_);
  file_menu_->addSeparator();
  file_menu_->addAction(exit_act_);

  view_menu_ = menuBar()->addMenu(tr("&View"));
  view_menu_->addAction(show_log_act_);

  QAction* open_local_action = new QAction(tr("Open &Local File..."), this);
  open_local_action->setStatusTip(tr("Open a local binary file"));
  connect(open_local_action, &QAction::triggered, connection_manager_,
          &ConnectionManager::openLocalFileDialog);

  help_menu_ = menuBar()->addMenu(tr("&Help"));
  help_menu_->addAction(about_act_);
  help_menu_->addAction(about_qt_act_);
}

void VelesMainWindow::showLog() {
  if (log_dock_widget_ == nullptr) {
    createLogWindow();
  }

  log_dock_widget_->raise();

  if (log_dock_widget_->window()->isMinimized()) {
    log_dock_widget_->window()->showNormal();
  }

  log_dock_widget_->window()->raise();
}

void VelesMainWindow::createDb() {
  qCritical() << "veles.mainwindow: createDb called";
  auto localShim = connection_manager_->localDbifShim();
  qCritical() << "  localShim:" << (localShim ? "valid" : "null");
  if (localShim) {
    qCritical() << "  isMapped:" << localShim->isMapped() << " size:" << localShim->size();
  }
  if (localShim && localShim->isMapped()) {
    auto wrapper = new client::LocalDbifWrapper(localShim, this);
    database_ = QSharedPointer<client::LocalDbifObjectHandle>::create(
        wrapper, *data::NodeID::getRootNodeId(), dbif::ObjectType::ROOT);
    createHexEditTab(localShim->filePath(), database_);
  }
}

void VelesMainWindow::createFileBlob(const QString& file_name) {
  data::BinData data(8, 0);

  if (!file_name.isEmpty()) {
    QFile file(file_name);
    file.setFileName(file_name);
    if (!file.open(QIODevice::ReadOnly)) {
      QMessageBox::warning(
          this, tr("Failed to open"),
          QString(tr("Failed to open \"%1\".")).arg(file_name));
      return;
    }
    QByteArray bytes = file.readAll();
    if (bytes.size() == 0 && file.size() != bytes.size()) {
      QMessageBox::warning(
          this, tr("File too large"),
          QString(tr("Failed to open \"%1\" due to current size limitation."))
              .arg(file_name));
      return;
    }
    data = data::BinData(8, bytes.size(),
                         reinterpret_cast<uint8_t*>(bytes.data()));
  }
  auto promise =
      database_->asyncRunMethod<dbif::RootCreateFileBlobFromDataRequest>(
          this, data, file_name);
  connect(promise, &dbif::MethodResultPromise::gotResult,
          [this, file_name](dbif::PMethodReply reply) {
            createHexEditTab(
                file_name.isEmpty() ? "untitled" : file_name,
                reply
                    .dynamicCast<
                        dbif::RootCreateFileBlobFromDataRequest::ReplyType>()
                    ->object);
          });

  connect(promise, &dbif::MethodResultPromise::gotError,
          [this, file_name](dbif::PError error) {
            QMessageBox::warning(this, tr("Veles"),
                                 tr("Cannot load file %1.").arg(file_name));
          });
}

void VelesMainWindow::createLogWindow() {
  auto* dock_widget = wrapWithDock(new LogWidget, "Log");
  dock_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  auto children = findChildren<DockWidget*>();
  if (!children.empty()) {
    tabifyDockWidget(children.back(), dock_widget);
  } else {
    addDockWidget(Qt::LeftDockWidgetArea, dock_widget);
  }

  log_dock_widget_ = dock_widget;
  QApplication::processEvents();
  updateDocksAndTabs();
}

}  // namespace ui
}  // namespace veles
