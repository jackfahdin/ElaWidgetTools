#include "ElaIconButton.h"

#include "ElaApplication.h"
#include <QEvent>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>

#include "ElaTheme.h"
#include "private/ElaIconButtonPrivate.h"
ELA_PROPERTY_Q_CPP(ElaIconButton, int, BorderRadius)
ELA_PROPERTY_Q_CPP(ElaIconButton, qreal, Opacity);
void ElaIconButton::setLightHoverColor(const QColor& color)
{
    Q_D(ElaIconButton);
    d->_pLightHoverColor = color;
    update();
    Q_EMIT pLightHoverColorChanged();
}

const QColor& ElaIconButton::getLightHoverColor() const
{
    Q_D(const ElaIconButton);
    return d->_pLightHoverColor.isValid() ? d->_pLightHoverColor : ElaThemeColor(ElaThemeType::Light, BasicHoverAlpha);
}

void ElaIconButton::setDarkHoverColor(const QColor& color)
{
    Q_D(ElaIconButton);
    d->_pDarkHoverColor = color;
    update();
    Q_EMIT pDarkHoverColorChanged();
}

const QColor& ElaIconButton::getDarkHoverColor() const
{
    Q_D(const ElaIconButton);
    return d->_pDarkHoverColor.isValid() ? d->_pDarkHoverColor : ElaThemeColor(ElaThemeType::Dark, BasicHoverAlpha);
}

void ElaIconButton::setLightIconColor(const QColor& color)
{
    Q_D(ElaIconButton);
    d->_pLightIconColor = color;
    update();
    Q_EMIT pLightIconColorChanged();
}

const QColor& ElaIconButton::getLightIconColor() const
{
    Q_D(const ElaIconButton);
    return d->_pLightIconColor.isValid() ? d->_pLightIconColor : ElaThemeColor(ElaThemeType::Light, BasicText);
}

void ElaIconButton::setDarkIconColor(const QColor& color)
{
    Q_D(ElaIconButton);
    d->_pDarkIconColor = color;
    update();
    Q_EMIT pDarkIconColorChanged();
}

const QColor& ElaIconButton::getDarkIconColor() const
{
    Q_D(const ElaIconButton);
    return d->_pDarkIconColor.isValid() ? d->_pDarkIconColor : ElaThemeColor(ElaThemeType::Dark, BasicText);
}

void ElaIconButton::setLightHoverIconColor(const QColor& color)
{
    Q_D(ElaIconButton);
    d->_pLightHoverIconColor = color;
    update();
    Q_EMIT pLightHoverIconColorChanged();
}

const QColor& ElaIconButton::getLightHoverIconColor() const
{
    Q_D(const ElaIconButton);
    return d->_pLightHoverIconColor.isValid() ? d->_pLightHoverIconColor
                                              : ElaThemeColor(ElaThemeType::Light, BasicText);
}

void ElaIconButton::setDarkHoverIconColor(const QColor& color)
{
    Q_D(ElaIconButton);
    d->_pDarkHoverIconColor = color;
    update();
    Q_EMIT pDarkHoverIconColorChanged();
}

const QColor& ElaIconButton::getDarkHoverIconColor() const
{
    Q_D(const ElaIconButton);
    return d->_pDarkHoverIconColor.isValid() ? d->_pDarkHoverIconColor : ElaThemeColor(ElaThemeType::Dark, BasicText);
}

ELA_PROPERTY_Q_CPP(ElaIconButton, bool, IsSelected);
ElaIconButton::ElaIconButton(const QPixmap& pix, QWidget* parent)
    : QPushButton(parent), d_ptr(new ElaIconButtonPrivate())
{
    Q_D(ElaIconButton);
    d->q_ptr = this;
    d->_iconPix = pix.copy();
    d->_pHoverAlpha = 0;
    d->_pOpacity = 1;
    d->_pIsSelected = false;
    d->_pBorderRadius = 0;
    d->_themeMode = eTheme->getThemeMode();
    connect(this, &ElaIconButton::pIsSelectedChanged, this, [=]() {
        update();
    });
    connect(eTheme, &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode themeMode) {
        d->_themeMode = themeMode;
    });
}

ElaIconButton::ElaIconButton(ElaIconType::IconName awesome, QWidget* parent)
    : QPushButton(parent), d_ptr(new ElaIconButtonPrivate())
{
    Q_D(ElaIconButton);
    d->q_ptr = this;
    d->_pHoverAlpha = 0;
    d->_pOpacity = 1;
    d->_pIsSelected = false;
    d->_pBorderRadius = 0;
    d->_themeMode = eTheme->getThemeMode();
    QFont iconFont = QFont("ElaAwesome");
    iconFont.setPixelSize(eApp->getFontPixelSize() + 2);
    this->setFont(iconFont);
    d->_pAwesome = awesome;
    this->setText(QChar(static_cast<char16_t>(awesome)));
    connect(this, &ElaIconButton::pIsSelectedChanged, this, [=]() {
        update();
    });
    connect(eTheme, &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode themeMode) {
        d->_themeMode = themeMode;
    });
}

ElaIconButton::ElaIconButton(ElaIconType::IconName awesome, int pixelSize, QWidget* parent)
    : QPushButton(parent), d_ptr(new ElaIconButtonPrivate())
{
    Q_D(ElaIconButton);
    d->q_ptr = this;
    d->_pHoverAlpha = 0;
    d->_pOpacity = 1;
    d->_pIsSelected = false;
    d->_pBorderRadius = 0;
    d->_themeMode = eTheme->getThemeMode();
    QFont iconFont = QFont("ElaAwesome");
    iconFont.setPixelSize(pixelSize);
    this->setFont(iconFont);
    d->_pAwesome = awesome;
    this->setText(QChar(static_cast<char16_t>(awesome)));
    connect(this, &ElaIconButton::pIsSelectedChanged, this, [=]() {
        update();
    });
    connect(eTheme, &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode themeMode) {
        d->_themeMode = themeMode;
    });
}

ElaIconButton::ElaIconButton(ElaIconType::IconName awesome, int pixelSize, int fixedWidth, int fixedHeight,
                             QWidget* parent)
    : QPushButton(parent), d_ptr(new ElaIconButtonPrivate())
{
    Q_D(ElaIconButton);
    d->q_ptr = this;
    d->_pHoverAlpha = 0;
    d->_pOpacity = 1;
    d->_pIsSelected = false;
    d->_pBorderRadius = 0;
    d->_themeMode = eTheme->getThemeMode();
    QFont iconFont = QFont("ElaAwesome");
    iconFont.setPixelSize(pixelSize);
    this->setFont(iconFont);
    d->_pAwesome = awesome;
    this->setText(QChar(static_cast<char16_t>(awesome)));
    this->setFixedSize(fixedWidth, fixedHeight);
    connect(this, &ElaIconButton::pIsSelectedChanged, this, [=]() {
        update();
    });
    connect(eTheme, &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode themeMode) {
        d->_themeMode = themeMode;
    });
}

ElaIconButton::~ElaIconButton()
{
}

void ElaIconButton::setAwesome(ElaIconType::IconName awesome)
{
    Q_D(ElaIconButton);
    d->_pAwesome = awesome;
    this->setText(QChar(static_cast<char16_t>(awesome)));
}

ElaIconType::IconName ElaIconButton::getAwesome() const
{
    return this->d_ptr->_pAwesome;
}

void ElaIconButton::setPixmap(const QPixmap& pix)
{
    Q_D(ElaIconButton);
    d->_iconPix = pix.copy();
}

bool ElaIconButton::event(QEvent* event)
{
    Q_D(ElaIconButton);
    switch (event->type())
    {
    case QEvent::Enter: {
        if (isEnabled() && !d->_pIsSelected)
        {
            d->_isAlphaAnimationFinished = false;
            QPropertyAnimation* alphaAnimation = new QPropertyAnimation(d, "pHoverAlpha", this);
            connect(alphaAnimation, &QPropertyAnimation::valueChanged, this, [=](const QVariant& value) {
                update();
            });
            connect(alphaAnimation, &QPropertyAnimation::finished, this, [=]() {
                d->_isAlphaAnimationFinished = true;
            });
            alphaAnimation->setDuration(175);
            alphaAnimation->setStartValue(d->_pHoverAlpha);
            alphaAnimation->setEndValue(d->_themeMode == ElaThemeType::Light ? getLightHoverColor().alpha()
                                                                             : getDarkHoverColor().alpha());
            alphaAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        }
        break;
    }
    case QEvent::Leave: {
        if (isEnabled() && !d->_pIsSelected)
        {
            d->_isAlphaAnimationFinished = false;
            QPropertyAnimation* alphaAnimation = new QPropertyAnimation(d, "pHoverAlpha", this);
            connect(alphaAnimation, &QPropertyAnimation::valueChanged, this, [=](const QVariant& value) {
                update();
            });
            connect(alphaAnimation, &QPropertyAnimation::finished, this, [=]() {
                d->_isAlphaAnimationFinished = true;
            });
            alphaAnimation->setDuration(175);
            alphaAnimation->setStartValue(d->_pHoverAlpha);
            alphaAnimation->setEndValue(0);
            alphaAnimation->start(QAbstractAnimation::DeleteWhenStopped);
        }
        break;
    }
    default: {
        break;
    }
    }
    return QPushButton::event(event);
}

void ElaIconButton::paintEvent(QPaintEvent* event)
{
    Q_D(ElaIconButton);
    QPainter painter(this);
    painter.save();
    painter.setOpacity(d->_pOpacity);
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setPen(Qt::NoPen);
    if (d->_isAlphaAnimationFinished || d->_pIsSelected)
    {
        painter.setBrush(d->_pIsSelected
                             ? d->_themeMode == ElaThemeType::Light ? getLightHoverColor() : getDarkHoverColor()
                         : isEnabled() ? underMouse() ? d->_themeMode == ElaThemeType::Light ? getLightHoverColor()
                                                                                             : getDarkHoverColor()
                                                      : Qt::transparent
                                       : Qt::transparent);
    }
    else
    {
        QColor hoverColor = d->_themeMode == ElaThemeType::Light ? getLightHoverColor() : getDarkHoverColor();
        hoverColor.setAlpha(d->_pHoverAlpha);
        painter.setBrush(hoverColor);
    }
    painter.drawRoundedRect(rect(), d->_pBorderRadius, d->_pBorderRadius);
    // 图标绘制
    if (!d->_iconPix.isNull())
    {
        QPainterPath path;
        path.addEllipse(rect());
        painter.setClipPath(path);
        painter.drawPixmap(rect(), d->_iconPix);
    }
    else
    {
        painter.setPen(isEnabled() ? d->_themeMode == ElaThemeType::Light
                                         ? underMouse() ? getLightHoverIconColor() : getLightIconColor()
                                     : underMouse() ? getDarkHoverIconColor()
                                                    : getDarkIconColor()
                                   : ElaThemeColor(d->_themeMode, BasicTextDisable));
        painter.drawText(rect(), Qt::AlignCenter, QChar(static_cast<char16_t>(d->_pAwesome)));
    }
    painter.restore();
}
