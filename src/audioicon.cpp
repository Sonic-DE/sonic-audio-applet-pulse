#include "audioicon.h"

#include <QMap>
#include <QString>

using namespace Qt::Literals::StringLiterals;

const QMap<QString, QString> formFactorMap = {
    {"internal"_L1, "audio-card"_L1},
    {"speaker"_L1, "audio-speakers-symbolic"_L1},
    {"phone"_L1, "phone"_L1},
    {"handset"_L1, "phone"_L1},
    {"tv"_L1, "video-television"_L1},
    {"webcam"_L1, "camera-web"_L1},
    {"microphone"_L1, "audio-input-microphone"_L1},
    {"headset"_L1, "audio-headset"_L1},
    {"headphone"_L1, "audio-headphones"_L1},
    {"hands-_L1free", "hands-free"_L1},
    {"car"_L1, "car"_L1},
    {"hifi"_L1, "hifi"_L1},
    {"computer"_L1, "computer"_L1},
    {"portable"_L1, "portable"_L1},
};

QString AudioIcon::forFormFactor(QString formFactor)
{
    return formFactorMap.value(formFactor, "");
}

QString AudioIcon::forVolume(int percent, bool muted, QString prefix)
{
    if (prefix.isEmpty()) {
        prefix = "audio-volume"_L1;
    }
    if (percent <= 0 || muted) {
        return prefix + "-muted";
    } else if (percent <= 25) {
        return prefix + "-low";
    } else if (percent <= 75) {
        return prefix + "-medium";
    } else if (percent <= HIGH_UPPER_BOUND) {
        return prefix + "-high";
    } else if (percent <= VERY_HIGH_UPPER_BOUND) {
        return prefix + "-high-warning";
    } else {
        return prefix + "-high-danger";
    }
}
