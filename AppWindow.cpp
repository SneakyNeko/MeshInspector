#include <QtWidgets>
#include "AppWindow.h"
#include "Histogram.h"
#include "StatsTable.h"
#include "MeshView.h"

AppWindow::AppWindow() : QMainWindow()
{
    setWindowTitle("Mesh Inspector");
    appData = new NodeList();
    setupMenu();
    setupAppTabs();
    statusBar()->showMessage("No mesh loaded");
}

AppWindow::AppWindow(const QString & file) : QMainWindow()
{
    setWindowTitle("Mesh Inspector");
    appData = new NodeList();
    setupMenu();
    setupAppTabs();
    setWindowFilePath(file);
    appData->loadFile(file);
    statusBar()->showMessage(QString("Loaded mesh: %1").arg(file));
}

void AppWindow::setupMenu()
{
    QMenu * fileMenu = new QMenu("&File", this);
    QMenu * prefMenu = new QMenu("P&references", this);
    
    fileMenu->addAction("&Open", this, SLOT (open()), QKeySequence::Open);
    fileMenu->addAction("&Save", this, SLOT (save()), QKeySequence::Save);
    fileMenu->addAction("Save &As", this, SLOT (saveAs()), QKeySequence::SaveAs);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit", QApplication::instance(), SLOT (quit()), QKeySequence::Quit);
    
    prefMenu->addAction("&Projection", this, SLOT (getProjection()));
    prefMenu->addAction("Polar &Data", appData, SLOT (setUnits(bool)))->setCheckable(true);
    prefMenu->addSeparator();
    prefMenu->addAction("&Logarithmic", appData, SIGNAL (updateScales(bool)))->setCheckable(true);
    
    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(prefMenu);
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
    setWindowFilePath(file);
    appData->loadFile(file);
    statusBar()->showMessage(QString("Loaded mesh: %1").arg(file));
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

