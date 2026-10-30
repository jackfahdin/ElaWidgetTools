#include "ElaApplication.h"
#include "ElaIcon.h"

#include <QFontDatabase>
#include <QImage>
#include <QtTest>

class ResourcesTest : public QObject
{
    Q_OBJECT
    QStringList _originalFontFamilies;
private Q_SLOTS:
    void initTestCase()
    {
        _originalFontFamilies = qApp->font().families();
        eApp->init();
    }

    void nonWindowsKeepsApplicationFont()
    {
#ifndef Q_OS_WIN
        QCOMPARE(qApp->font().families(), _originalFontFamilies);
#else
        QSKIP("Windows 保留原有默认字体");
#endif
    }

    void fontsAndImagesAreLinked()
    {
        QVERIFY(QFile::exists(":/include/Font/ElaAwesome.ttf"));
        QVERIFY(QFontDatabase::families().contains("ElaAwesome"));
        QVERIFY(!QImage(":/include/Image/Cirno.jpg").isNull());
    }

    void iconCanBeRendered()
    {
        const auto icon = ElaIcon::getInstance()->getElaIcon(ElaIconType::House, 24, Qt::black);
        const auto pixmap = icon.pixmap(32, 32);
        QVERIFY(!pixmap.isNull());
        const auto image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
        bool hasInk = false;
        for (int y = 0; y < image.height() && !hasInk; ++y)
        {
            for (int x = 0; x < image.width(); ++x)
            {
                if (qAlpha(image.pixel(x, y)) != 0)
                {
                    hasInk = true;
                    break;
                }
            }
        }
        QVERIFY(hasInk);
    }
};

QTEST_MAIN(ResourcesTest)
#include "tst_resources.moc"
