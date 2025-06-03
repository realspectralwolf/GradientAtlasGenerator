#include "mainwindow.h"
#include <QPixmap>
#include <QImage>

#include <QColorDialog>
#include <QColor>
#include <QPushButton>
#include <QFileDialog>

int margin = 100;
int resolution = 64;
int displayRes = 512;
int columns = 4;

QColor LerpColor(QColor first, QColor second, float t)
{
    float r = first.redF() + (second.redF() - first.redF()) * t;
    float g = first.greenF() + (second.greenF() - first.greenF()) * t;
    float b = first.blueF() + (second.blueF() - first.blueF()) * t;
    return QColor::fromRgbF(r, g, b);
}

const char * randomColors[] =
    {
    "#ff4583",
    "#ac474f",
    "#ff8548",
    "#ffe066",
    "#9eed5c",
    "#74bf4e",
    "#726aa3",
    "#2db3c1"
    };

void GradientFill(QImage &img, int startX, int endX, int startY, int endY, QColor a, QColor b)
{
    for (int y = startY; y < endY; y++)
    {
        QColor c = LerpColor(a, b, y / ((float)resolution));
        for (int x = startX; x < endX; x++)
        {
            img.setPixelColor(x, y, c);
        }
    }
}

void MainWindow::RedrawTexture()
{
    image.fill(QColor(20, 20, 20));

    int width = resolution / columns;
    for (int i = 0 ; i < columns; i++)
    {
        GradientFill(image, i * width, (i+1) * width, 0, resolution, colors[i * 2], colors[i * 2 + 1]);
    }

    QPixmap pixmap = QPixmap();
    pixmap = QPixmap::fromImage(image);
    pixmap = pixmap.scaled(displayRes, displayRes);
    imageLabel->setPixmap(pixmap);
}

void MainWindow::AddButton(int colorIndex, int ax, int ay, int aw, int ah)
{
    // Button
    QPushButton *button = new QPushButton("🎨", this);
    button->setGeometry(ax, ay, aw, ah);

    // Connect signal
    connect(button, &QPushButton::clicked, [this, colorIndex]() {
        QColorDialog *dialog = new QColorDialog(Qt::white, this);
        dialog->setWindowTitle("Select Color");
        dialog->move(100, 200);

        QColor prevColor = colors[colorIndex];

        // Live update: fires every time the color changes
        connect(dialog, &QColorDialog::currentColorChanged, this, [this, colorIndex](const QColor &color) {
            colors[colorIndex] = color;
            RedrawTexture();
        });

        // Final confirmed color
        connect(dialog, &QColorDialog::colorSelected, this, [this, prevColor, colorIndex](const QColor &color) {
            colors[colorIndex] = color;
            RedrawTexture();
        });

        // Cancelled
        connect(dialog, &QDialog::rejected, this, [this, prevColor, colorIndex]() {
            colors[colorIndex] = prevColor;
            RedrawTexture();
        });

        dialog->open();  // non-blocking, shows dialog
    });
}

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), imageLabel(new QLabel(this))
{
    image = QImage(resolution, resolution, QImage::Format_RGB32);

    colors.clear();
    for (int i = 0; i < columns * 2; i++)
    {
        colors.push_back(randomColors[i]);
    }

    imageLabel->setAlignment(Qt::AlignCenter);
    setCentralWidget(imageLabel);

    int bWidth = displayRes / columns;
    int bHeight = 30;
    for (int i = 0; i < columns; i++)
    {
        AddButton(i*2 + 0, margin + i * bWidth, margin - bHeight - 6, bWidth, bHeight);
        AddButton(i*2 + 1, margin + i * bWidth, margin + displayRes + 6, bWidth, bHeight);
    }

    QPushButton *exportButton = new QPushButton("Export", this);
    exportButton->setGeometry((displayRes + margin * 2 - 100) / 2, displayRes + margin + 50, 100, 40);

    // Connect signal
    connect(exportButton, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this,
            "Save File",
            QDir::homePath() + "/gradient_atlas.png",
            "Images (*.png);;All Files (*)");

        if (!fileName.isEmpty()) {
            qDebug() << "Save to file:" << fileName;
            bool saveState = image.save(fileName);
            qDebug() << "Success?" << saveState;
        }
    });

    QSize windowSize = QSize(512 + margin * 2, 512 + margin * 2);
    resize(windowSize);

    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    RedrawTexture();
}

MainWindow::~MainWindow() {}
