/****************************************************************************
 *   Copyright (C) 2011  Andreas Kling <awesomekling@gmail.com>             *
 *   Copyright (C) 2011  Instituto Nokia de Tecnologia (INdT)               *
 *                                                                          *
 *   This file may be used under the terms of the GNU Lesser                *
 *   General Public License version 2.1 as published by the Free Software   *
 *   Foundation and appearing in the file LICENSE.LGPL included in the      *
 *   packaging of this file.  Please review the following information to    *
 *   ensure the GNU Lesser General Public License version 2.1 requirements  *
 *   will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.   *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Lesser General Public License for more details.                    *
 ****************************************************************************/

#include "PageWidget.h"

#include "BrowserWindow.h"
#include "DeclarativeDesktopWebView.h"

#include <QtDeclarative/QDeclarativeItem>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QLineEdit>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>

#include <QUrl>

// Keeping this feature commented out until we have support for it in the new API.
//QWKPage* newPageCallback(QWKPage* parentPage)
//{
//    BrowserWindow* window = BrowserWindow::create(parentPage->context());
//    PageWidget* page = window->openNewTab();
//    window->show();
//    return page->page();
//}

PageWidget::PageWidget(QWidget* parent)
    : QDeclarativeView(parent)
    , m_tabWidget(0)
{
    setResizeMode(QDeclarativeView::SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/main.qml"));

    m_root = qobject_cast<QDeclarativeItem*>(rootObject());
    Q_ASSERT(m_root);

    m_tabWidget = m_root->findChild<QDeclarativeItem*>("tabWidget");
    Q_ASSERT(m_tabWidget);

    connect(m_tabWidget, SIGNAL(tabAdded(QVariant)), this, SLOT(onTabAdded(QVariant)));
    connect(m_tabWidget, SIGNAL(currentTabChanged()), this, SLOT(onCurrentTabChanged()));

    onTabAdded(m_tabWidget->property("currentActiveTab"));

    QAction* focusLocationBarAction = new QAction(this);
    focusLocationBarAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    connect(focusLocationBarAction, SIGNAL(triggered()), this, SLOT(focusUrlBarRequested()));
    addAction(focusLocationBarAction);

    QAction* newTabAction = new QAction(this);
    newTabAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    connect(newTabAction, SIGNAL(triggered()), this, SLOT(newTabRequested()));
    addAction(newTabAction);

    QAction* closeTabAction = new QAction(this);
    closeTabAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
    connect(closeTabAction, SIGNAL(triggered()), this, SLOT(closeTabRequested()));
    addAction(closeTabAction);

//    page()->setCreateNewPageFunction(newPageCallback);
}

PageWidget::~PageWidget()
{
}

void PageWidget::openInNewTab(const QString& urlFromUserInput)
{
    QUrl url = QUrl::fromUserInput(urlFromUserInput);
    if (!url.isEmpty()) {
        QObject* currentActiveTab = m_tabWidget->property("currentActiveTab").value<QObject*>();
        QObject* mainView = currentActiveTab->property("mainView").value<QObject*>();
        QObject* urlEdit = mainView->findChild<QDeclarativeItem*>("urlEdit");
        urlEdit->setProperty("text", url.toString());
        getWebViewForUrlEdit(urlEdit)->setUrl(url);
    }
}

void PageWidget::jumpToNextTab()
{
    QMetaObject::invokeMethod(m_tabWidget, "jumpToNextTab");
}

void PageWidget::jumpToPreviousTab()
{
    QMetaObject::invokeMethod(m_tabWidget, "jumpToPreviousTab");
}

void PageWidget::onTabAdded(QVariant tab)
{
    QObject* mainView = tab.value<QObject*>()->property("mainView").value<QObject*>();
    QDeclarativeItem* urlEdit = mainView->findChild<QDeclarativeItem*>("urlEdit");
    connect(urlEdit, SIGNAL(urlEntered(QString)), this, SLOT(onUrlChanged(QString)));
    QObject* webView = getWebViewForUrlEdit(urlEdit);
    connect(webView, SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));
}

void PageWidget::onCurrentTabChanged()
{
    QObject* currentActiveTab = m_tabWidget->property("currentActiveTab").value<QObject*>();
    emit titleChanged(currentActiveTab->property("text").toString());
}

void PageWidget::onTitleChanged(const QString& title)
{
    emit titleChanged(title);
}

DeclarativeDesktopWebView* PageWidget::getWebViewForUrlEdit(QObject* urlEdit)
{
    QObject* view = urlEdit->property("view").value<QObject*>();
    return qobject_cast<DeclarativeDesktopWebView* >(view);
}

void PageWidget::onUrlChanged(const QString& url)
{
    getWebViewForUrlEdit(sender())->setUrl(QUrl::fromUserInput(url));
}

void PageWidget::newTabRequested()
{
    QMetaObject::invokeMethod(m_tabWidget, "addNewTab");
}

void PageWidget::closeTabRequested()
{
    QMetaObject::invokeMethod(m_tabWidget, "closeTab", Q_ARG(QVariant, m_tabWidget->property("currentActiveTab")));
}

void PageWidget::focusUrlBarRequested()
{
    QObject* currentActiveTab = m_tabWidget->property("currentActiveTab").value<QObject*>();
    QObject* mainView = currentActiveTab->property("mainView").value<QObject*>();
    QMetaObject::invokeMethod(mainView, "focusUrlBar");
}
