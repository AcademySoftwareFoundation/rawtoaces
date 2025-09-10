#include <QtTest/QtTest>
#include <QApplication>
#include "../src/utils/ImageUtils.h"
#include "../src/ParameterWidget.h"

class BasicGuiTests : public QObject {
    Q_OBJECT
private slots:
    void imageutils_placeholder();
    void imageutils_frame_size();
    void parameterwidget_selection_mapping();
};

void BasicGuiTests::imageutils_placeholder()
{
    QImage img = ImageUtils::loadPreview("/path/does/not/exist.raw", 128, 96);
    QVERIFY(!img.isNull());
    QCOMPARE(img.width(), 128);
}

void BasicGuiTests::imageutils_frame_size()
{
    QImage framed = ImageUtils::loadFramedThumbnail("/path/does/not/exist.raw", 96, 72);
    QCOMPARE(framed.size(), QSize(96,72));
}

void BasicGuiTests::parameterwidget_selection_mapping()
{
    ParameterWidget w;
    QRect r(10, 20, 30, 40);
    w.setWbBoxFromSelection(r);
    auto params = w.getParameters();
    QCOMPARE(params.wbMethod, QString("box"));
    QCOMPARE(params.wbBoxOrigin, QPoint(10,20));
    QCOMPARE(params.wbBoxSize, QSize(30,40));
}

QTEST_MAIN(BasicGuiTests)
#include "test_basic.moc"
