// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2026 SonicDE contributors

#include <QSignalSpy>
#include <QtTest/QTest>

#include "microphonetest.h"
#include <limits>
#include <pulse/operation.h>

// Source lookup operations are simulated so these lifecycle tests need no audio
// server or microphone. Only the operation API is replaced; rate validation is
// performed by libpulse itself.
struct pa_operation {
    bool cancelled = false;
    int references = 1;
};

extern "C" void pa_operation_cancel(pa_operation* operation)
{
    operation->cancelled = true;
}

extern "C" void pa_operation_unref(pa_operation* operation)
{
    --operation->references;
}

class MicrophoneLifecycleTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void pendingLookupIsRecording()
    {
        pa_operation operation;
        MicrophoneTest test;
        test.m_sourceInfoOperation = &operation;
        test.m_recordedData = QByteArray("old recording");

        QVERIFY(test.recording());
        // Both actions must be blocked while waiting for source information.
        test.startRecording();
        test.playRecording();
        QVERIFY(test.recording());
        QVERIFY(!test.playing());
        QCOMPARE(test.m_sourceInfoOperation, &operation);
    }

    void cancelLookup_data()
    {
        QTest::addColumn<int>("action");
        QTest::newRow("stop") << 0;
        QTest::newRow("cleanup") << 1;
        QTest::newRow("clear") << 2;
    }

    void cancelLookup()
    {
        QFETCH(int, action);
        pa_operation operation;
        MicrophoneTest test;
        test.m_sourceInfoOperation = &operation;
        QSignalSpy recordingChanged(&test, &MicrophoneTest::recordingChanged);

        switch (action) {
        case 0:
            test.stopRecording();
            break;
        case 1:
            test.cleanupStreams();
            break;
        case 2:
            test.clearRecording();
            break;
        }

        QVERIFY(operation.cancelled);
        QCOMPARE(operation.references, 0);
        QVERIFY(!test.recording());
        QCOMPARE(recordingChanged.count(), 1);

        // A cancelled/stale result cannot restart recording or change the rate.
        pa_source_info info{};
        info.sample_spec.rate = 96000;
        MicrophoneTest::source_info_callback(nullptr, &info, 0, &test);
        MicrophoneTest::source_info_callback(nullptr, nullptr, 1, &test);
        QCOMPARE(test.m_pendingSampleRate, 44100U);
        QCOMPARE(operation.references, 0);
    }

    void destructionCancelsLookup()
    {
        pa_operation operation;
        {
            MicrophoneTest test;
            test.m_sourceInfoOperation = &operation;
        }
        QVERIFY(operation.cancelled);
        QCOMPARE(operation.references, 0);
    }

    void lookupResult_data()
    {
        QTest::addColumn<uint32_t>("rate");
        QTest::addColumn<uint32_t>("expectedRate");
        QTest::addColumn<int>("eol");
        QTest::newRow("native-48k") << 48000U << 48000U << 1;
        QTest::newRow("native-96k") << 96000U << 96000U << 1;
        QTest::newRow("invalid-zero") << 0U << 44100U << 1;
        QTest::newRow("invalid-too-large") << std::numeric_limits<uint32_t>::max() << 44100U << 1;
        QTest::newRow("async-error") << 0U << 44100U << -1;
    }

    void lookupResult()
    {
        QFETCH(uint32_t, rate);
        QFETCH(uint32_t, expectedRate);
        QFETCH(int, eol);
        pa_operation operation;
        MicrophoneTest test;
        test.m_sourceInfoOperation = &operation;
        test.m_sampleRate = 22050;
        test.m_recordedData = QByteArray("old recording");
        QSignalSpy recordingChanged(&test, &MicrophoneTest::recordingChanged);

        pa_source_info info{};
        info.sample_spec.rate = rate;
        if (eol > 0) {
            MicrophoneTest::source_info_callback(nullptr, &info, 0, &test);
            QVERIFY(test.recording());
            QCOMPARE(operation.references, 1);
        }
        QCOMPARE(test.m_pendingSampleRate, expectedRate);

        MicrophoneTest::source_info_callback(nullptr, nullptr, eol, &test);
        QCOMPARE(operation.references, 0);
        QVERIFY(!operation.cancelled);
        QVERIFY(!test.recording());
        QCOMPARE(recordingChanged.count(), 1);
        // No source is selected, so stream setup fails. The old audio must
        // retain its original rate, even after a successful native-rate lookup.
        QCOMPARE(test.m_sampleRate, 22050U);
        QCOMPARE(test.m_recordedData, QByteArray("old recording"));
    }
};

QTEST_GUILESS_MAIN(MicrophoneLifecycleTest)

#include "microphonelifecycletest.moc"
