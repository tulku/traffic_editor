/*
 * Copyright (C) 2019 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include "level_dialog.h"
#include <QtWidgets>


LevelDialog::LevelDialog(QWidget *parent, Level &_level)
: QDialog(parent),
  level(_level)
{
  ok_button = new QPushButton("OK", this);  // first button = [enter] button
  cancel_button = new QPushButton("Cancel", this);

  QHBoxLayout *name_hbox_layout = new QHBoxLayout;
  name_line_edit = new QLineEdit(QString::fromStdString(level.name), this);
  name_hbox_layout->addWidget(new QLabel("name:"));
  name_hbox_layout->addWidget(name_line_edit);

  QHBoxLayout *drawing_filename_layout = new QHBoxLayout;
  drawing_filename_line_edit = new QLineEdit(
      QString::fromStdString(level.drawing_filename), this);
  drawing_filename_button = new QPushButton("Find...", this);
  drawing_filename_layout->addWidget(new QLabel("drawing:"));
  drawing_filename_layout->addWidget(drawing_filename_line_edit);
  drawing_filename_layout->addWidget(drawing_filename_button);
  connect(
      drawing_filename_button, &QAbstractButton::clicked,
      this, &LevelDialog::drawing_filename_button_clicked);
  connect(
      drawing_filename_line_edit,
      &QLineEdit::textEdited,
      this,
      &LevelDialog::drawing_filename_line_edited);

  QHBoxLayout *instr_layout = new QHBoxLayout;
  instr_layout->addWidget(new QLabel(
      "Explicit dimensions are only needed if drawing is not provided:"));

  QHBoxLayout *x_hbox_layout = new QHBoxLayout;
  x_line_edit = new QLineEdit(QString::number(level.x_meters), this);
  x_hbox_layout->addWidget(new QLabel("x dimension (meters):"));
  x_hbox_layout->addWidget(x_line_edit);

  QHBoxLayout *y_hbox_layout = new QHBoxLayout;
  y_line_edit = new QLineEdit(QString::number(level.y_meters), this);
  y_hbox_layout->addWidget(new QLabel("y dimension (meters):"));
  y_hbox_layout->addWidget(y_line_edit);

  QHBoxLayout *bottom_buttons_layout = new QHBoxLayout;
  bottom_buttons_layout->addWidget(cancel_button);
  bottom_buttons_layout->addWidget(ok_button);
  connect(
      ok_button, &QAbstractButton::clicked,
      this, &LevelDialog::ok_button_clicked);
  connect(
      cancel_button, &QAbstractButton::clicked,
      this, &QDialog::reject);

  QVBoxLayout *vbox_layout = new QVBoxLayout;
  vbox_layout->addLayout(name_hbox_layout);
  vbox_layout->addLayout(drawing_filename_layout);
  vbox_layout->addLayout(instr_layout);
  vbox_layout->addLayout(x_hbox_layout);
  vbox_layout->addLayout(y_hbox_layout);
  // todo: some sort of separator (?)
  vbox_layout->addLayout(bottom_buttons_layout);

  setLayout(vbox_layout);

  enable_dimensions(level.drawing_filename.empty());
}

LevelDialog::~LevelDialog()
{
}

void LevelDialog::drawing_filename_button_clicked()
{
  QFileDialog file_dialog(this, "Find Drawing");
  file_dialog.setFileMode(QFileDialog::ExistingFile);
  file_dialog.setNameFilter("*.png");
  if (file_dialog.exec() != QDialog::Accepted) {
    if (drawing_filename_line_edit->text().isEmpty())
      enable_dimensions(true);
    return;  // user clicked 'cancel'
  }
  const QString filename = file_dialog.selectedFiles().first();
  if (!QFileInfo(filename).exists()) {
    QMessageBox::critical(
        this,
        "Drawing file does not exist",
        "File does not exist.");
    if (drawing_filename_line_edit->text().isEmpty())
      enable_dimensions(true);
    return;
  }
  drawing_filename_line_edit->setText(
      QDir::current().relativeFilePath(filename));
  enable_dimensions(false);
}

void LevelDialog::ok_button_clicked()
{
  if (!drawing_filename_line_edit->text().isEmpty()) {
    // make sure the drawing file exists
    if (!QFileInfo(drawing_filename_line_edit->text()).exists()) {
      QMessageBox::critical(
          this,
          "If supplied, drawing filename must exist",
          "If supplied, drawing filename must exist");
    }
    return;
  }
  /*
  // todo: figure out how to test for valid numeric values;
  // this doesn't work but there must be a similar function somewhere
  if (!x_line_edit->text().isNumber() || !y_line_edit->text().isNumber()) {
    QMessageBox::critical(
        this,
        "X and Y dimensions must be numbers",
        "X and Y dimensions must be numbers");
    return;
  }
  */
  if (name_line_edit->text().isEmpty()) {
    QMessageBox::critical(
        this,
        "Name must not be empty",
        "Name must not be empty");
    return;
  }
  level.name = name_line_edit->text().toStdString();
  level.drawing_filename = drawing_filename_line_edit->text().toStdString();
  if (level.drawing_filename.empty()) {
    level.x_meters = x_line_edit->text().toDouble();
    level.y_meters = y_line_edit->text().toDouble();
  }
  else {
    level.x_meters = 0.0;
    level.y_meters = 0.0;
  }
  level.calculate_scale();
  accept();
}

void LevelDialog::enable_dimensions(const bool enable)
{
  if (enable) {
    x_line_edit->setEnabled(true);
    y_line_edit->setEnabled(true);
  }
  else {
    x_line_edit->setText("10");
    y_line_edit->setText("10");
    x_line_edit->setEnabled(false);
    y_line_edit->setEnabled(false);
  }
}

void LevelDialog::drawing_filename_line_edited(const QString &text)
{
  enable_dimensions(text.isEmpty());
}
