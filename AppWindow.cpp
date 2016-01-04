#include <QtWidgets>
#include "AppWindow.h"
#include "Histogram.h"
#include "StatsTable.h"
#include "MeshView.h"

#include <stdio.h>

AppWindow::AppWindow() : QMainWindow()
{
    setWindowTitle("Mesh Inspector");
    appData = new NodeList();
    setupMenu();
    setupAppTabs();
    statusBar()->showMessage("No mesh loaded");
}

void AppWindow::setupMenu()
{
    QMenu * fileMenu = new QMenu("&File", this);
    QMenu * prefMenu = new QMenu("&Preferences", this);
    QMenu * statMenu = new QMenu("&Export Stats", this);
    
    fileMenu->addAction("&Open...", this, SLOT (open()), QKeySequence::Open);
    fileMenu->addAction("&Save", this, SLOT (save()), QKeySequence::Save);
    fileMenu->addAction("Save &As...", this, SLOT (saveAs()), QKeySequence::SaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit", QApplication::instance(), SLOT (quit()), QKeySequence::Quit);
    
    prefMenu->addAction("P&rojection...", this, SLOT (getProjection()));
    polarStats = prefMenu->addAction("Polar &Data", appData, SLOT (setUnits(bool)));
    polarStats->setCheckable(true);
    prefMenu->addSeparator();
    prefMenu->addAction("&Logarithmic", appData, SIGNAL (updateScales(bool)))->setCheckable(true);
    
    statMenu->addAction("Save depths", this, SLOT (saveDepth()));
    statMenu->addSection("Maximum");
    statMenu->addAction("Save max areas", this, SLOT (saveMaxArea()));
    statMenu->addAction("Save max side lengths", this, SLOT (saveMaxLength()));
    statMenu->addAction("Save max dh/h", this, SLOT (saveMaxDH()));
    statMenu->addAction("Save max dt", this, SLOT (saveMaxDT()));
    statMenu->addSection("Minimum");
    statMenu->addAction("Save min areas", this, SLOT (saveMinArea()));
    statMenu->addAction("Save min side lengths", this, SLOT (saveMinLength()));
    statMenu->addAction("Save min dh/h", this, SLOT (saveMinDH()));
    statMenu->addAction("Save min dt", this, SLOT (saveMinDT()));
 
    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(prefMenu);
    menuBar()->addMenu(statMenu);
}

void AppWindow::setupAppTabs()
{
    QList<QString> tabs;
    QString tab;
    QTabWidget * widget = new QTabWidget(this);
    
    tabs << "Dh/h" << "Area" << "Depth" << "Side" << "dt";
    foreach (tab, tabs) {
        Histogram  * hist  = new Histogram(tab, widget);
        QTableView * table = new QTableView(widget);
        StatsTable * stats = new StatsTable(tab, appData, table);
        hist->setModel(appData);
        table->setModel(stats);
        table->setAlternatingRowColors(true);
        table->horizontalHeader()->setStretchLastSection(true);
        table->setFixedHeight(51);
        table->setFrameShape(QFrame::NoFrame);
        
        QWidget     * page   = new QWidget(widget);
        QVBoxLayout * layout = new QVBoxLayout(page);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(table, 0);
        layout->addWidget(hist, 1);
        page->setLayout(layout);
        widget->addTab(page, tab);
        
        connect(appData, SIGNAL (listUpdated()), hist, SLOT (updateData()));
        connect(appData, SIGNAL (updateScales(bool)), hist, SLOT (setScale(bool)));
        connect(appData, SIGNAL (listUpdated()), table, SLOT (reset()));
    }
    
    MeshView * mesh = new MeshView(widget);
    mesh->setModel(appData);
    widget->addTab(mesh, "Map");
    
    setCentralWidget(widget);
}

void AppWindow::open()
{
    QString file = QFileDialog::getOpenFileName(this, "Open Mesh",
                                            windowFilePath(),
                                            NodeList::fileFilter);
    open(file);
}

void AppWindow::open(const QString file)
{
    if (file.isEmpty()) return;
    QDir path(file);
    
    setWindowFilePath(path.absolutePath());
    appData->loadFile(path.absolutePath());
    statusBar()->showMessage(QString("Loaded mesh: %1").arg(path.absolutePath()));
}

void AppWindow::save()
{
    QString file = windowFilePath();
    
    if (!file.isEmpty())
        appData->saveFile(file);
    else
        saveAs();
}

void AppWindow::saveAs()
{
    QString file = QFileDialog::getSaveFileName(this, "Mesh Save File",
                                                windowFilePath(),
                                                NodeList::fileFilter);
    setWindowFilePath(file);
    appData->saveFile(file);
    statusBar()->showMessage(QString("Saved mesh: %1").arg(file));
}

void AppWindow::getProjection()
{
    bool ok = false;
    QString proj = QInputDialog::getText(this, "Coordinate Projection",
                                         "libProj compatable projection:",
                                         QLineEdit::Normal, appData->projection(), &ok);
    
    if (ok && !proj.isEmpty()) appData->setProjection(proj);
}

void AppWindow::saveDepth()
{
    QString file(windowFilePath());
    if (file.isEmpty()) return;
    
    file.chop(4);
    file.append("_depth.s2r");
    print_depth(appData->data(), qPrintable(file));
}

void AppWindow::saveMaxArea()
{
    QString file(windowFilePath());
    if (file.isEmpty()) return;
    
    file.chop(4);
    file.append("_max_area.s2r");
    print_general(appData->data(), polarStats->isChecked(), 0, EXPORT_AREA, qPrintable(file));
}

void AppWindow::saveMaxLength()
{
    QString file(windowFilePath());
    if (file.isEmpty()) return;
    
    file.chop(4);
    file.append("_max_side.s2r");
    print_side(appData->data(), polarStats->isChecked(), 0, qPrintable(file));
}

void AppWindow::saveMaxDH()
{
    QString file(windowFilePath());
    if (file.isEmpty()) return;
    
    file.chop(4);
    file.append("_max_delta_h.s2r");
    print_general(appData->data(), polarStats->isChecked(), 0, EXPORT_D_H, qPrintable(file));
}

void AppWindow::saveMaxDT()
{
    QString file(windowFilePath());
    if (file.isEmpty()) return;
    
    file.chop(4);
    file.append("_max_delta_t.s2r");
    print_general(appData->data(), polarStats->isChecked(), 0, EXPORT_D_T, qPrintable(file));
}

void AppWindow::saveMinArea()
{
    QString file(windowFilePath());
    if (file.isEmpty()) return;
    
    file.chop(4);
    file.append("_min_area.s2r");
    print_general(appData->data(), polarStats->isChecked(), 1, EXPORT_AREA, qPrintable(file));
}

void AppWindow::saveMinLength()
{
    QString file(windowFilePath());
    if (file.isEmpty()) return;
    
    file.chop(4);
    file.append("_min_side.s2r");
    print_side(appData->data(), polarStats->isChecked(), 1, qPrintable(file));
}

void AppWindow::saveMinDH()
{
    QString file(windowFilePath());
    if (file.isEmpty()) return;
    
    file.chop(4);
    file.append("_min_delta_h.s2r");
    print_general(appData->data(), polarStats->isChecked(), 1, EXPORT_D_H, qPrintable(file));
}

void AppWindow::saveMinDT()
{
    QString file(windowFilePath());
    if (file.isEmpty()) return;
    
    file.chop(4);
    file.append("_min_delta_t.s2r");
    print_general(appData->data(), polarStats->isChecked(), 1, EXPORT_D_T, qPrintable(file));
}

