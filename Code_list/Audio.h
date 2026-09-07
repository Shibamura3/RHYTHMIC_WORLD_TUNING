#pragma once // 最終編集日 2026/08/30
enum AudioType
{
    AUDIO_BGM,
    AUDIO_SE,
    AUDIO_VOICE
};

void InitAudio();
void UninitAudio();

int LoadAudio(const char* FileName, AudioType type);
void UnloadAudio(int Index);

void PlayAudio(int Index, bool Loop = false); // 再生

bool PlayAudioFromTime(int Index, float startTime, bool Loop = false); // 指定秒数から再生
float GetAudioPlaybackTime(int Index); // 現在の再生位置を秒で取得
float GetAudioDuration(int Index); // 音源の総再生時間を秒で取得

void SetAudioVolume(int Index, float volume);
void ApplyVolumeSetting(int master, int bgm, int se, int voice);

void StopAudio(int Index); // 停止
void PauseAudio(int Index);
void ResumeAudio(int Index);

bool IsPlaying(int Index);