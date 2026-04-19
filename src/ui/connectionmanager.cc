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
#include "ui/connectionmanager.h"

#include <QFileDialog>
#include <QTextStream>

#include "client/localdbifshim.h"
#include "ui/logwidget.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* ConnectionManager */
/*****************************************************************************/

ConnectionManager::ConnectionManager(QWidget* parent) : QObject(parent) {
}

ConnectionManager::~ConnectionManager() {
  delete localDbifShim_;
}

client::LocalDbifShim* ConnectionManager::localDbifShim() {
  return localDbifShim_;
}

void ConnectionManager::openLocalFile(const QString& filePath) {
  delete localDbifShim_;
  localDbifShim_ = new client::LocalDbifShim(filePath, this);

  if (localDbifShim_->isMapped()) {
    QTextStream out(LogWidget::output());
    out << "LocalDbifShim: Opened file " << filePath << " size:"
        << localDbifShim_->size() << endl;
    emit localFileOpened(filePath);
  } else {
    QString errorMsg = localDbifShim_->errorString();
    QTextStream out(LogWidget::output());
    out << "LocalDbifShim: Failed to open file " << filePath << ":"
        << errorMsg << endl;
    emit localFileOpenFailed(errorMsg);
  }
}

void ConnectionManager::openLocalFileDialog() {
  QString filePath = QFileDialog::getOpenFileName(
      nullptr,
      tr("Open Binary File"),
      QString(),
      tr("All Files (*.*)"));

  if (!filePath.isEmpty()) {
    openLocalFile(filePath);
  }
}

}  // namespace ui
}  // namespace veles