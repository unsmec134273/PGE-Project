/*
 * Platformer Game Engine by Wohlstand, a free platform for game making
 * Copyright (c) 2014 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


#include "../ui_mainwindow.h"
#include "../mainwindow.h"



//Reload opened file data
void MainWindow::on_actionReload_triggered()
{
    LevelData FileData;
    QString filePath;
    QRect wnGeom;

    if (activeChildWindow()==1)
    {
        filePath = activeLvlEditWin()->curFile;

        QFile fileIn(filePath);

        if (!fileIn.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("File open error"),
        tr("Can't open the file."), QMessageBox::Ok);
            return;
        }

        FileData = ReadLevelFile(fileIn); //function in file_formats.cpp
        if( !FileData.ReadFileValid ){
            statusBar()->showMessage(tr("Reloading error"), 2000);
            return;}

        FileData.filename = QFileInfo(filePath).baseName();
        FileData.path = QFileInfo(filePath).absoluteDir().absolutePath();
        FileData.playmusic = autoPlayMusic;
        activeLvlEditWin()->LvlData.modified = false;
        activeLvlEditWin()->close();
        wnGeom = ui->centralWidget->activeSubWindow()->geometry();
        ui->centralWidget->activeSubWindow()->close();

        leveledit *child = createChild();
        if ((bool) (child->loadFile(filePath, FileData, configs, LvlOpts))) {
            statusBar()->showMessage(tr("Level file reloaded"), 2000);
            child->show();
            ui->centralWidget->activeSubWindow()->setGeometry(wnGeom);
            SetCurrentLevelSection(0);
        } else {
                WriteToLog(QtDebugMsg, ">>File loading aborted");
            child->show();
                WriteToLog(QtDebugMsg, ">>Window showed");
            if(activeChildWindow()==1) activeLvlEditWin()->LvlData.modified = false;
                WriteToLog(QtDebugMsg, ">>Option seted");
            ui->centralWidget->activeSubWindow()->close();
                WriteToLog(QtDebugMsg, ">>Windows closed");
        }
    }
}


void MainWindow::on_actionExport_to_image_triggered()
{
    if(activeChildWindow()==1)
    {
        activeLvlEditWin()->ExportToImage_fn();
    }
}

void MainWindow::on_actionReset_position_triggered()
{
    if (activeChildWindow()==1)
    {
       activeLvlEditWin()->ResetPosition();
    }
}



void MainWindow::on_actionSelect_triggered()
{
    ui->actionSelect->setChecked(1);
    ui->actionEriser->setChecked(0);
    ui->actionHandScroll->setChecked(0);

    ui->actionSetFirstPlayer->setChecked(0);
    ui->actionSetSecondPlayer->setChecked(0);
    ui->actionDrawWater->setChecked(0);
    ui->actionDrawSand->setChecked(0);

    if ((activeChildWindow()==1) && (ui->actionSelect->isChecked()))
    {
       activeLvlEditWin()->changeCursor(0);
       activeLvlEditWin()->scene->EditingMode = 0;
       activeLvlEditWin()->scene->EraserEnabled = false;
    }
}

void MainWindow::on_actionEriser_triggered()
{
    ui->actionSelect->setChecked(0);
    ui->actionEriser->setChecked(1);
    ui->actionHandScroll->setChecked(0);

    ui->actionSetFirstPlayer->setChecked(0);
    ui->actionSetSecondPlayer->setChecked(0);
    ui->actionDrawWater->setChecked(0);
    ui->actionDrawSand->setChecked(0);

    if ((activeChildWindow()==1) && (ui->actionEriser->isChecked()))
    {
       activeLvlEditWin()->changeCursor(1);
       activeLvlEditWin()->scene->EditingMode = 1;
       activeLvlEditWin()->scene->EraserEnabled = false;
    }
}


void MainWindow::on_actionHandScroll_triggered()
{
    ui->actionSelect->setChecked(0);
    ui->actionEriser->setChecked(0);
    ui->actionHandScroll->setChecked(1);

    ui->actionSetFirstPlayer->setChecked(0);
    ui->actionSetSecondPlayer->setChecked(0);
    ui->actionDrawWater->setChecked(0);
    ui->actionDrawSand->setChecked(0);

    if ((activeChildWindow()==1) && (ui->actionHandScroll->isChecked()))
    {
       activeLvlEditWin()->scene->clearSelection();
       activeLvlEditWin()->changeCursor(-1);
       activeLvlEditWin()->scene->EditingMode = 0;
       activeLvlEditWin()->scene->EraserEnabled = false;
    }
}


void MainWindow::on_actionAnimation_triggered(bool checked)
{
    LvlOpts.animationEnabled = checked;
    if (activeChildWindow()==1)
    {
        activeLvlEditWin()->scene->opts.animationEnabled = LvlOpts.animationEnabled;
        if(LvlOpts.animationEnabled)
        {
            activeLvlEditWin()->scene->startBlockAnimation();
        }
        else
            activeLvlEditWin()->scene->stopAnimation();
    }
}


void MainWindow::on_actionCollisions_triggered(bool checked)
{
    LvlOpts.collisionsEnabled = checked;
    if (activeChildWindow()==1)
    {
        activeLvlEditWin()->scene->opts.collisionsEnabled = LvlOpts.collisionsEnabled;
    }

}



