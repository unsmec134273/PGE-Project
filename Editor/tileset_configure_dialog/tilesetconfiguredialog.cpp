/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2014 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tilesetconfiguredialog.h"
#include "ui_tilesetconfiguredialog.h"

TilesetConfigureDialog::TilesetConfigureDialog(dataconfigs* conf, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TilesetConfigureDialog)
{
    ui->setupUi(this);
    ui->tilesetLayoutWidgetContainer->insertWidget(0,m_tileset = (new tileset(conf,tileset::WORLDTILESET_TILE,0,64,3,3)));

    ui->listView->setAcceptDrops(true);
    ui->listView->setDropIndicatorShown(true);
    ui->listView->setDragEnabled(true);
    ui->listView->setModel(m_model = (new PiecesModel(conf, PiecesModel::WORLDPIECE_TILE)));

    m_conf = conf;
    for(int i = 0; i < conf->main_wtiles.size(); ++i){
        m_model->addPiece(i);
    }

    connect(ui->spin_width,SIGNAL(valueChanged(int)),m_tileset,SLOT(setCols(int)));
    connect(ui->spin_height,SIGNAL(valueChanged(int)),m_tileset,SLOT(setRows(int)));

}

TilesetConfigureDialog::~TilesetConfigureDialog()
{
    delete ui;
}

void TilesetConfigureDialog::on_pushButton_clicked()
{
    m_tileset->clear();
}
