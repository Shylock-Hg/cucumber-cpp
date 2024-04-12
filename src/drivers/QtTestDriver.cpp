#include "cucumber-cpp/internal/drivers/QtTestDriver.hpp"

#include <QtTest/QtTest>
#include <QTextStream>
#include <QTemporaryFile>

namespace cucumber {
namespace internal {


// wraps the QTemporaryFile creation
// on Windows the file could not be written as long as QTemporaryFile owner of the file.
class TemporaryFileWrapper {
public:
    static TemporaryFileWrapper create()
    {
        QTemporaryFile tempFile{};

        if(!tempFile.open()) {
            return {};
        }

        return {tempFile.fileName() + ".txt"};
    }

    TemporaryFileWrapper(): filename{} {
    }

    TemporaryFileWrapper(QString filename): filename{filename} {
    }

    ~TemporaryFileWrapper() {
        QFile::remove(filename);
    }

    bool exists() const { return !filename.isEmpty(); }

    QString name() const {
        return filename;
    }

    QString read() const {
        QFile file{ filename };
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return QString();
        QTextStream in(&file);
        return in.readAll();
    }

private:
    QString filename;
};

const InvokeResult QtTestStep::invokeStepBody() {
    const auto file = TemporaryFileWrapper::create();
    if (!file.exists()) {
        return InvokeResult::failure("Unable to open temporary file needed for this test");
    }

    QtTestObject testObject{this};
    const QStringList args{"test", "-o", file.name() + ",tap"};
    const int returnValue = QTest::qExec(&testObject, args);

    if (returnValue == 0) {
        return InvokeResult::success();
    } else {
        return InvokeResult::failure(file.read().toLocal8Bit());
    }
}

}
}
