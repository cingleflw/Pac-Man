/**
 * @file sound_manager.cpp
 * @brief Реализация SoundManager.
 */

#include "sound_manager.hpp"

#include <iostream>
#include <utility>

bool SoundManager::init() {
  // Подсистему аудио поднимаем явно: открытие устройства её требует, а окно
  // инициализирует только видео.
  if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
    std::cerr << "SoundManager: SDL_InitSubSystem(AUDIO) failed: "
              << SDL_GetError() << std::endl;
    return false;
  }

  // Открываем устройство по умолчанию и получаем РЕАЛЬНЫЙ id. Привязывать поток
  // к константе SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK нельзя — она лишь «пожелание»
  // для открытия. nullptr — формат выберет SDL.
  dev_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
  if (!dev_) {
    std::cerr << "SoundManager: SDL_OpenAudioDevice failed: " << SDL_GetError()
              << std::endl;
    return false;
  }

  // В ручном (bind) пути устройство и так стартует без паузы; вызов
  // оборонительный — он no-op, если пауза уже снята.
  SDL_ResumeAudioDevice(dev_);
  return true;
}

bool SoundManager::load_wav(SoundType type, const std::string& path) {
  SDL_AudioSpec spec;
  Uint8* buffer = nullptr;
  Uint32 length = 0;
  if (!SDL_LoadWAV(path.c_str(), &spec, &buffer, &length)) {
    std::cerr << "SoundManager: SDL_LoadWAV failed for " << path << ": "
              << SDL_GetError() << std::endl;
    return false;
  }

  // Поток с НЕнулевым src-форматом этого файла. Нулевой src + поздняя установка
  // формата — известный путь к тишине, поэтому формат задаём сразу. dst равен
  // src лишь номинально: bind ниже переустановит выходной формат под
  // устройство.
  SDL_AudioStream* stream = SDL_CreateAudioStream(&spec, &spec);
  if (!stream) {
    std::cerr << "SoundManager: SDL_CreateAudioStream failed: "
              << SDL_GetError() << std::endl;
    SDL_free(buffer);
    return false;
  }

  // Привязка к ОТКРЫТОМУ устройству. Она же выставляет выходной формат потока
  // под устройство, так что данные сконвертируются автоматически.
  if (!SDL_BindAudioStream(dev_, stream)) {
    std::cerr << "SoundManager: SDL_BindAudioStream failed: " << SDL_GetError()
              << std::endl;
    SDL_DestroyAudioStream(stream);
    SDL_free(buffer);
    return false;
  }

  Voice v;
  v.stream = stream;
  v.buffer = buffer;
  v.length = length;
  // move-assign освободит прежний голос этого типа, если load_wav вызван
  // повторно.
  voices_[type] = std::move(v);
  return true;
}

void SoundManager::play(SoundType type) {
  auto it = voices_.find(type);
  if (it == voices_.end()) return;

  // Глобальные события заглушают остальное, чтобы не мешалось.
  if (type == SoundType::Start || type == SoundType::Death ||
      type == SoundType::Victory) {
    stop_all();
  }

  Voice& v = it->second;
  // Рестарт с начала: вычищаем очередь голоса и заливаем данные заново.
  SDL_ClearAudioStream(v.stream);
  SDL_PutAudioStreamData(v.stream, v.buffer, v.length);
}

void SoundManager::stop_all() {
  for (auto& [type, v] : voices_) {
    if (v.stream) SDL_ClearAudioStream(v.stream);
  }
}

void SoundManager::clean() {
  // Уничтожаем потоки и буферы (деструкторы Voice) ПОКА SDL ещё жив.
  voices_.clear();
  if (dev_) {
    SDL_CloseAudioDevice(dev_);
    dev_ = 0;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  }
}
