#pragma once

enum class PlaybackState {
    Idle,
    Connecting,
    Buffering,
    Playing,
    Reconnecting,
    Error
};