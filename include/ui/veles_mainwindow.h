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
#pragma once

#include <QDropEvent>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QString>
#include <QStringList>

#include "dbif/promise.h"
#include "dbif/types.h"
#include "ui/connectionmanager.h"
#include "ui/dialogs/optionsdialog.h"
#include "ui/mainwindowwithdetachabledockwidgets.h"
#include "ui/shortcutssettings.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* VelesMainWindow */
/*****************************************************************************/

class VelesMainWindow : public MainWindowWithDetachableDockWidgets {
  Q_OBJECT

 public:
   VelesMainWindow();
   void addFile(const QString& path);

  protected:
  void dropEvent(QDropEvent* ev) override;
  void dragEnterEvent(QDragEnterEvent* ev) override;
  void showEvent(QShowEvent* event) override;

  private slots:
   void newFile();
   void open();
   void about();
   void openLocalFile();
   void showLog();

 signals:
  void shown();

 private:
  void init();
  void createActions();
  void createMenus();
  void createDb();
  void createFileBlob(const QString& file_name);
  void createLogWindow();

  QMenu* file_menu_;
  QMenu* view_menu_;
  QMenu* help_menu_;

  QAction* new_file_act_;
  QAction* open_act_;
  QAction* exit_act_;
  QAction* options_act_;
  QAction* shortcuts_act_;

  QAction* about_act_;
  QAction* about_qt_act_;

  QAction* show_log_act_;

  dbif::ObjectHandle database_;
   OptionsDialog* options_dialog_;
   ShortcutsDialog* shortcuts_dialog_;

   QPointer<DockWidget> log_dock_widget_;

  ConnectionManager* connection_manager_;
};

}  // namespace ui
}  // namespace veles
