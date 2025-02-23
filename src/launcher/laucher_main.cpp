#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QDir>
#include <QProcess>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QStringList>
#include <QDebug>

const QString kSearchPaths[] = {
    ".conda/envs",
    "micromamba/envs",
};

bool isTargetPythonVersion(const QString &pypath);

QString readConfigFile(const QString &fileName)
{
    QSettings settings(fileName, QSettings::IniFormat);
    return settings.value("Python/Path", "").toString();
}

void searchDirectoryForPython(const QDir &dir, QStringList &paths)
{
    qDebug() << "searchDirectoryForPython " << dir.path();

    for (const QFileInfo &entry : dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
    {
        if (!entry.isDir())
        {
            continue;
        }

        QString pypath = QDir(entry.absoluteFilePath()).absoluteFilePath("python.exe");
        qDebug() << "searchDirectoryForPython try: " << pypath;
        if (!pypath.isEmpty())
        {
            if (isTargetPythonVersion(pypath))
            {
                paths.append(entry.absoluteFilePath());
            }
        }
    }
}

QStringList searchPythonPaths()
{
    QStringList paths;
    int psize = sizeof(kSearchPaths) / sizeof(kSearchPaths[0]);
    qDebug() << "Searching " << psize;
    for (int i = 0; i < psize; i++)
    {
        QDir userDir(QDir::home().absoluteFilePath(kSearchPaths[i]));
        searchDirectoryForPython(userDir, paths);
    }

    return paths;
}

QString getPythonVersion(const QString pypath)
{
    QProcess process;
    process.setWorkingDirectory(QDir::temp().path());
    process.start(pypath, {"--version"});
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QString version = output.trimmed();
    return version;
}

bool hasPython312(const QString &version_str)
{
    return version_str.contains("Python 3.12");
}

bool hasPython312InSys()
{
    return hasPython312(getPythonVersion("python"));
}

bool isTargetPythonVersion(const QString &pypath)
{
    return hasPython312(getPythonVersion(pypath));
}

// 显示选择对话框，支持编辑
QString showPythonPathSelectionDialog(const QStringList &paths)
{
    QDialog dialog;
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *label = new QLabel("Please select or enter the path to your Python 3.12:");
    layout->addWidget(label);

    QComboBox *comboBox = new QComboBox;
    comboBox->setEditable(true);
    comboBox->addItems(paths);
    comboBox->lineEdit()->setPlaceholderText("example: C:/Users/myname/micromamba/envs/py312");
    layout->addWidget(comboBox);

    QPushButton *button = new QPushButton("OK");
    layout->addWidget(button);

    QObject::connect(button, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted)
    {
        return comboBox->currentText();
    }

    return "";
}

void putInputPythonToEnv(const QString &pypath)
{
    QString env_path = qgetenv("PATH");
    if (!env_path.contains(pypath))
    {
        qDebug() << "Add python path to PATH environment variable:" << env_path;
        env_path = pypath + ";" + env_path;
        qputenv("PATH", env_path.toLocal8Bit().data());
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!hasPython312InSys())
    {
        QString configFileName = "config.txt";
        QString pythonPath = readConfigFile(configFileName);

        if (pythonPath.isEmpty())
        {
            QStringList foundPaths = searchPythonPaths();
            if (foundPaths.isEmpty())
            {
                qDebug() << "cannot find python 3.12 in default search path!";
            }

            pythonPath = showPythonPathSelectionDialog(foundPaths);
            if (pythonPath.isEmpty())
            {
                QMessageBox::critical(nullptr, "Error", "No Python 3.12 selected or input.");
                return -1;
            }
            else if (!isTargetPythonVersion(QDir(pythonPath).absoluteFilePath("python.exe")))
            {
                QMessageBox::critical(nullptr, "Error", "No Python 3.12 in the input path.");
                return -1;
            }
            else
            {
                QSettings settings(configFileName, QSettings::IniFormat);
                settings.setValue("Python/Path", pythonPath);
            }
        }
        qDebug() << "Selected Python 3.12 Path:" << pythonPath;
        putInputPythonToEnv(pythonPath);
    }
    else
    {
        qDebug() << "Python 3.12 is already installed in sys.";
    }

    QStringList arguments;
    QProcess::startDetached("QppCAD.exe", arguments);

    QCoreApplication::exit();

    return 0;
}