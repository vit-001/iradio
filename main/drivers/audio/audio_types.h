#pragma once

enum class PlaybackState {
    Idle,
    Connecting,
    Playing,
    Reconnecting,
    Paused
};