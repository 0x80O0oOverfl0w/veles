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

#include <memory>

#include <QAction>
#include <QFileDialog>
#include <QObject>
#include <QString>

#include "client/localdbifshim.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* ConnectionManager */
/*****************************************************************************/

class ConnectionManager : public QObject {
  Q_OBJECT

 public:
  explicit ConnectionManager(QWidget* parent = nullptr);
  ~ConnectionManager() override;

  client::LocalDbifShim* localDbifShim();

  bool isLocalMode() const { return localDbifShim_ != nullptr && localDbifShim_->isMapped(); }

 signals:
  void localFileOpened(const QString& filePath);
  void localFileOpenFailed(const QString& error);

 public slots:
  void openLocalFile(const QString& filePath);
  void openLocalFileDialog();

 private:
  client::LocalDbifShim* localDbifShim_ = nullptr;
};

}  // namespace ui
}  // namespace veles