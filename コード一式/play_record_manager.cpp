/*
    プレイ情報の管理：play_record_manager.h

    2026/08/24	hibiki sakuma
*/
#include "play_record_manager.h"
#include "Configu.h"

#include <direct.h>
#include <fstream>
#include <sstream>
#include <vector>

namespace
{
    struct SongRecord
    {
        std::string songId;

        DifficultyRecord records[static_cast<int>(GameDifficulty::Max)];
    };

    static std::vector<SongRecord> g_Records;

    static constexpr const char* SAVE_DIRECTORY = "save";

    static constexpr const char* SAVE_FILE_PATH = "save/play_records.csv";


    static int FindRecordIndex(const std::string& songId){
        for (int i = 0; i < static_cast<int>(g_Records.size()); ++i){
            if (g_Records[i].songId == songId){
                return i;
            }
        }

        return -1;
    }


    static SongRecord& GetOrCreateSongRecord(const std::string& songId){
        const int index = FindRecordIndex(songId);

        if (index >= 0){
            return g_Records[index];
        }

        SongRecord newRecord;
        newRecord.songId = songId;

        g_Records.push_back(newRecord);

        return g_Records.back();
    }
}


void PlayRecord_Initialize(){
    PlayRecord_Load();
}


void PlayRecord_Finalize(){
    PlayRecord_Save();

    g_Records.clear();
}


bool PlayRecord_Load(){
    g_Records.clear();

    std::ifstream file(SAVE_FILE_PATH);

    if (!file.is_open()){
        return true;
    }

    std::string line;

    // ヘッダーを読み飛ばす
    std::getline(file, line);

    while (std::getline(file, line)){
        if (line.empty()){
            continue;
        }

        std::stringstream stream(line);

        std::string songId;
        std::string difficultyText;
        std::string playedText;
        std::string clearedText;
        std::string scoreText;
        std::string rankText;

        std::getline(stream, songId, ',');

        std::getline(stream, difficultyText, ',');

        std::getline(stream, playedText, ',');

        std::getline(stream, clearedText, ',');

        std::getline(stream, scoreText, ',');

        std::getline( stream, rankText,',');

        try
        {
            const int difficultyIndex = std::stoi(difficultyText);

            if (difficultyIndex < 0 || difficultyIndex >=static_cast<int>(GameDifficulty::Max)){
                continue;
            }

            SongRecord& record = GetOrCreateSongRecord(songId);

            DifficultyRecord& difficultyRecord = record.records[difficultyIndex];

            difficultyRecord.hasPlayed = std::stoi(playedText) != 0;

            difficultyRecord.hasCleared = std::stoi(clearedText) != 0;

            difficultyRecord.highScore = std::stoi(scoreText);

            const int rank = std::stoi(rankText);

            if (rank >= Rank_D && rank < Rank_MAX){
                difficultyRecord.highRank = static_cast<Rank_Type>(rank);
            }
        }catch (const std::exception&){
            continue;
        }
    }

    return true;
}


bool PlayRecord_Save(){
    _mkdir(SAVE_DIRECTORY);

    std::ofstream file(SAVE_FILE_PATH, std::ios::trunc);

    if (!file.is_open()){
        return false;
    }

    file
        << "SONG_ID,DIFFICULTY,"
        << "HAS_PLAYED,HAS_CLEARED,"
        << "HIGH_SCORE,HIGH_RANK\n";

    for (const SongRecord& song : g_Records){
        for (int i = 0;i < static_cast<int>(GameDifficulty::Max); ++i){
            const DifficultyRecord& record = song.records[i];

            file
                << song.songId
                << ","
                << i
                << ","
                << (record.hasPlayed ? 1 : 0)
                << ","
                << (record.hasCleared ? 1 : 0)
                << ","
                << record.highScore
                << ","
                << static_cast<int>(record.highRank)
                << "\n";
        }
    }

    return true;
}


void PlayRecord_Update(const std::string& songId, GameDifficulty difficulty,int score, Rank_Type rank, bool hasCleared){
    if (songId.empty()){
        return;
    }

    const int difficultyIndex = static_cast<int>(difficulty);

    if (difficultyIndex < 0 || difficultyIndex >= static_cast<int>(GameDifficulty::Max)){
        return;
    }

    SongRecord& songRecord = GetOrCreateSongRecord(songId);

    DifficultyRecord& record = songRecord.records[difficultyIndex];

    record.hasPlayed = true;

    if (hasCleared){
        record.hasCleared = true;
    }

    if (score > record.highScore){
        record.highScore = score;
        record.highRank = rank;
    }

    PlayRecord_Save();
}


const DifficultyRecord* PlayRecord_Get(const std::string& songId, GameDifficulty difficulty){
    const int songIndex = FindRecordIndex(songId);

    if (songIndex < 0){
        return nullptr;
    }

    const int difficultyIndex = static_cast<int>(difficulty);

    if (difficultyIndex < 0 || difficultyIndex >= static_cast<int>(GameDifficulty::Max)){
        return nullptr;
    }

    return & g_Records[songIndex].records[difficultyIndex];
}