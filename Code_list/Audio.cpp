#include <xaudio2.h>
#include <assert.h>
#include "audio.h"

#pragma comment(lib, "xaudio2.lib")  // XAudio2Create のために必要
#pragma comment(lib, "winmm.lib")    // mmioOpen, mmioAscend などのために必要

static IXAudio2* g_Xaudio{};
static IXAudio2MasteringVoice* g_MasteringVoice{};
//IXAudio2 昔のプログラミング、調べて勉強
//デシベル→ログ計算が必要になる

struct AUDIO{
	IXAudio2SourceVoice* SourceVoice{};
	BYTE* SoundData{};

	int Length{};
	int PlayLength{};

	// WAVのサンプルレート
	UINT32 SamplesPerSecond{};

	// 今回の再生開始サンプル位置
	UINT32 PlayBeginSample{};

	// 再生開始時点のSamplesPlayed
	UINT64 SamplesPlayedAtStart{};

	// 再生中フラグ
	bool Playing{};

	AudioType Type{};
};

#define AUDIO_MAX 100
static AUDIO g_Audio[AUDIO_MAX]{};

void InitAudio() {
	// XAudio生成
	XAudio2Create(&g_Xaudio, 0);

	// マスタリングボイス生成
	g_Xaudio->CreateMasteringVoice(&g_MasteringVoice);
}


void UninitAudio() {
	g_MasteringVoice->DestroyVoice();
	g_Xaudio->Release();
}

int LoadAudio(const char *FileName, AudioType type){
	int index = -1;

	for (int i = 0; i < AUDIO_MAX; i++){
		if (g_Audio[i].SourceVoice == nullptr){
			index = i;
			break;
		}
	}

	if (index == -1)
		return -1;

	// サウンドデータ読込
	WAVEFORMATEX wfx = { 0 };

	{
		HMMIO hmmio = NULL;
		MMIOINFO mmioinfo = { 0 };
		MMCKINFO riffchunkinfo = { 0 };
		MMCKINFO datachunkinfo = { 0 };
		MMCKINFO mmckinfo = { 0 };
		UINT32 buflen;
		LONG readlen;


		hmmio = mmioOpen((LPSTR)FileName, &mmioinfo, MMIO_READ);
		assert(hmmio);

		riffchunkinfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		mmioDescend(hmmio, &riffchunkinfo, NULL, MMIO_FINDRIFF);

		mmckinfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
		mmioDescend(hmmio, &mmckinfo, &riffchunkinfo, MMIO_FINDCHUNK);

		if (mmckinfo.cksize >= sizeof(WAVEFORMATEX)){
			mmioRead(hmmio, (HPSTR)&wfx, sizeof(wfx));
		} else {
			PCMWAVEFORMAT pcmwf = { 0 };
			mmioRead(hmmio, (HPSTR)&pcmwf, sizeof(pcmwf));
			memset(&wfx, 0x00, sizeof(wfx));
			memcpy(&wfx, &pcmwf, sizeof(pcmwf));
			wfx.cbSize = 0;
		}
		mmioAscend(hmmio, &mmckinfo, 0);

		datachunkinfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
		mmioDescend(hmmio, &datachunkinfo, &riffchunkinfo, MMIO_FINDCHUNK);



		buflen = datachunkinfo.cksize;
		g_Audio[index].SoundData = new unsigned char[buflen];
		readlen = mmioRead(hmmio, (HPSTR)g_Audio[index].SoundData, buflen);


		g_Audio[index].Length = readlen;

		g_Audio[index].PlayLength = readlen / wfx.nBlockAlign;

		g_Audio[index].SamplesPerSecond = wfx.nSamplesPerSec;

		g_Audio[index].PlayBeginSample = 0;
		g_Audio[index].SamplesPlayedAtStart = 0;
		g_Audio[index].Playing = false;

		g_Audio[index].Type = type;

		mmioClose(hmmio, 0);
	}


	// サウンドソース生成
	g_Xaudio->CreateSourceVoice(&g_Audio[index].SourceVoice, &wfx);
	assert(g_Audio[index].SourceVoice);


	return index;
}

void UnloadAudio(int Index){
	AUDIO& audio = g_Audio[Index];

	audio.SourceVoice->Stop();
	audio.SourceVoice->FlushSourceBuffers();
	audio.SourceVoice->DestroyVoice();

	delete[] audio.SoundData;

	audio.SourceVoice = nullptr;
	audio.SoundData = nullptr;

	audio.Length = 0;
	audio.PlayLength = 0;
	audio.SamplesPerSecond = 0;
	audio.PlayBeginSample = 0;
	audio.SamplesPlayedAtStart = 0;
	audio.Playing = false;
}

void PlayAudio(int Index, bool Loop){
	PlayAudioFromTime(Index, 0.0f, Loop);
}

bool PlayAudioFromTime(int Index, float startTime, bool Loop){
	AUDIO& audio = g_Audio[Index];

	if (audio.SamplesPerSecond == 0 || audio.PlayLength <= 0){
		return false;
	}

	// マイナス時刻を防止
	if (startTime < 0.0f){
		startTime = 0.0f;
	}

	const float duration = static_cast<float>(audio.PlayLength) / static_cast<float>(audio.SamplesPerSecond);

	// 曲末尾以降からは再生できない
	if (startTime >= duration){
		audio.Playing = false;
		return false;
	}

	UINT32 playBeginSample = static_cast<UINT32>(startTime * static_cast<float>(audio.SamplesPerSecond));

	if (playBeginSample >= static_cast<UINT32>(audio.PlayLength)){
		playBeginSample = static_cast<UINT32>(audio.PlayLength - 1);
	}

	audio.SourceVoice->Stop();
	audio.SourceVoice->FlushSourceBuffers();

	XAUDIO2_VOICE_STATE state{};
	audio.SourceVoice->GetState(&state);

	audio.PlayBeginSample = playBeginSample;

	audio.SamplesPlayedAtStart = state.SamplesPlayed;

	XAUDIO2_BUFFER buffer{};

	buffer.AudioBytes = audio.Length;
	buffer.pAudioData =audio.SoundData;
	buffer.PlayBegin = playBeginSample;
	buffer.PlayLength = static_cast<UINT32>(audio.PlayLength) - playBeginSample;
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	if (Loop){
		buffer.LoopBegin = playBeginSample;
		buffer.LoopLength =static_cast<UINT32>(audio.PlayLength) - playBeginSample;
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

		// ループ時にEND_OF_STREAMは不要
		buffer.Flags = 0;
	}

	const HRESULT submitResult = audio.SourceVoice->SubmitSourceBuffer(&buffer, nullptr);

	if (FAILED(submitResult)){
		audio.Playing = false;
		return false;
	}

	const HRESULT startResult = audio.SourceVoice->Start();

	if (FAILED(startResult)){
		audio.SourceVoice->FlushSourceBuffers();
		audio.Playing = false;
		return false;
	}

	audio.Playing = true;

	return true;
}

float GetAudioPlaybackTime(int Index){
	AUDIO& audio = g_Audio[Index];

	if (audio.SamplesPerSecond == 0) return 0.0f;

	XAUDIO2_VOICE_STATE state{};
	audio.SourceVoice->GetState(&state);

	UINT64 playedSinceStart = 0;

	if (state.SamplesPlayed >= audio.SamplesPlayedAtStart){
		playedSinceStart = state.SamplesPlayed - audio.SamplesPlayedAtStart;
	}

	const UINT64 currentSample = static_cast<UINT64>(audio.PlayBeginSample) + playedSinceStart;

	float currentTime = static_cast<float>(currentSample) / static_cast<float>(audio.SamplesPerSecond);

	const float duration = GetAudioDuration(Index);

	if (currentTime > duration){
		currentTime = duration;
	}

	return currentTime;
}

float GetAudioDuration(int Index) {
	const AUDIO& audio = g_Audio[Index];

	if (audio.SamplesPerSecond == 0) return 0.0f;

	return
		static_cast<float>(audio.PlayLength) /
		static_cast<float>(audio.SamplesPerSecond);
}


void SetAudioVolume(int Index, float volume) {
	if (g_Audio[Index].SourceVoice == nullptr) return;

	g_Audio[Index].SourceVoice->SetVolume(volume);
}

void ApplyVolumeSetting(int master, int bgm, int se, int voice){
	for (int i = 0; i < AUDIO_MAX; i++){
		if (g_Audio[i].SourceVoice == nullptr) continue;

		float volume = master / 100.0f;

		switch (g_Audio[i].Type)
		{
		case AUDIO_BGM:
			volume *= bgm / 100.0f;
			break; 

		case AUDIO_SE:
			volume *= se / 100.0f;
			break;

		case AUDIO_VOICE:
			volume *= voice / 100.0f;
			break;
		}

		g_Audio[i].SourceVoice->SetVolume(volume);
	}

}


void StopAudio(int Index){
	if (g_Audio[Index].SourceVoice == nullptr) return;

	g_Audio[Index].SourceVoice->Stop();
}

void PauseAudio(int Index){
	if (g_Audio[Index].SourceVoice == nullptr) return;

	g_Audio[Index].SourceVoice->Stop();
}

void ResumeAudio(int Index){
	if (g_Audio[Index].SourceVoice == nullptr) return;

	g_Audio[Index].SourceVoice->Start();
}

bool IsPlaying(int Index){
	XAUDIO2_VOICE_STATE state;

	g_Audio[Index].SourceVoice->GetState(&state);

	return state.BuffersQueued > 0;
}
