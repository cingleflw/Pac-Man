/**
 * @file sound_manager.hpp
 * @brief Менеджер звука: по одному аудиопотоку на каждый тип звука, чтобы
 *        устройство микшировало их между собой.
 *
 * В SDL3 микширование происходит МЕЖДУ привязанными к устройству потоками, а
 * внутри одного потока данные складываются в очередь. Поэтому каждый SoundType
 * получает собственный SDL_AudioStream: разные звуки звучат одновременно, а
 * повторный запуск одного и того же звука перезапускает его (clear + put), не
 * накапливая отставание от игры.
 */

#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <unordered_map>

/// @brief Типы звуковых эффектов в игре.
enum class SoundType {
  Start,     ///< Музыка на старте уровня.
  EatDot,    ///< Поедание точки или энерджайзера.
  EatGhost,  ///< Поедание испуганного призрака.
  Death,     ///< Смерть Pac-Man.
  Victory,   ///< Победа (уровень очищен).
  Scare1,    ///< Звук скримера № 1.
  Scare2,    ///< Звук скримера № 2.
  Scare3,    ///< Звук скримера № 3.
  Scare4,    ///< Звук скримера № 4.
};

/**
 * @brief Синглтон-менеджер звука поверх SDL3 Audio (без SDL_mixer).
 *
 * Жизненный цикл: init() один раз после старта SDL -> load_wav() на каждый звук
 * -> play() по игровым событиям -> clean() ПЕРЕД SDL_Quit().
 *
 * @warning clean() обязательно вызывать вручную до SDL_Quit(). Менеджер живёт
 * как статический объект, и его деструктор отработал бы уже после завершения
 * SDL (use-after-quit). Поэтому деструктор намеренно НЕ освобождает ресурсы —
 * по образцу TextureManager.
 */
class SoundManager {
 public:
  /// @brief Единственный экземпляр менеджера.
  static SoundManager& instance() {
    static SoundManager inst;
    return inst;
  }

  SoundManager(const SoundManager&) = delete;
  SoundManager& operator=(const SoundManager&) = delete;

  /**
   * @brief Поднимает подсистему аудио и открывает устройство вывода.
   *
   * Потоки привязываются к устройству позже, в load_wav().
   *
   * @pre SDL инициализирован (достаточно созданного окна/рендерера).
   * @return true при успехе, false при ошибке открытия устройства.
   */
  bool init();

  /**
   * @brief Загружает WAV и заводит под него отдельный привязанный поток.
   *
   * Поток создаётся с РЕАЛЬНЫМ форматом этого файла, поэтому разные звуки могут
   * быть в разных частотах/каналах — каждый конвертируется в формат устройства
   * независимо. Повторный вызов с тем же type заменяет прежний звук, освобождая
   * его ресурсы.
   *
   * @param[in] type Идентификатор звука.
   * @param[in] path Путь к WAV-файлу.
   * @pre init() выполнен успешно.
   * @return true при успехе загрузки и привязки.
   */
  bool load_wav(SoundType type, const std::string& path);

  /**
   * @brief Проигрывает звук с начала.
   *
   * Чистит собственную очередь звука и заливает его заново, поэтому повторный
   * вызов перезапускает звук без накопления задержки. Разные SoundType звучат
   * одновременно (микшируются устройством). Глобальные события (Start, Death,
   * Victory) предварительно гасят все остальные звуки.
   *
   * @param[in] type Идентификатор звука.
   */
  void play(SoundType type);

  /// @brief Останавливает все звуки (например, на паузе игры).
  void stop_all();

  /// @brief Уничтожает потоки, освобождает буферы и закрывает устройство.
  void clean();

 private:
  SoundManager() = default;
  ~SoundManager() =
      default;  ///< НЕ чистит ресурсы: см. предупреждение к классу.

  /**
   * @brief Один озвучиваемый «голос»: привязанный поток + данные WAV.
   *
   * Владеет ресурсами SDL (RAII): поток и буфер освобождаются в деструкторе.
   * Копирование запрещено (иначе double-free), перемещение обнуляет источник —
   * это нужно, чтобы unordered_map мог переносить элементы при рехешировании.
   */
  struct Voice {
    SDL_AudioStream* stream = nullptr;  ///< Привязанный к устройству поток.
    Uint8* buffer = nullptr;            ///< Аудиоданные WAV в памяти.
    Uint32 length = 0;                  ///< Длина данных в байтах.

    Voice() = default;
    ~Voice() {
      if (stream) SDL_DestroyAudioStream(stream);
      if (buffer) SDL_free(buffer);
    }

    Voice(const Voice&) = delete;
    Voice& operator=(const Voice&) = delete;

    Voice(Voice&& o) noexcept
        : stream(o.stream), buffer(o.buffer), length(o.length) {
      o.stream = nullptr;
      o.buffer = nullptr;
      o.length = 0;
    }

    Voice& operator=(Voice&& o) noexcept {
      if (this != &o) {
        if (stream) SDL_DestroyAudioStream(stream);
        if (buffer) SDL_free(buffer);
        stream = o.stream;
        buffer = o.buffer;
        length = o.length;
        o.stream = nullptr;
        o.buffer = nullptr;
        o.length = 0;
      }
      return *this;
    }
  };

  SDL_AudioDeviceID dev_ = 0;  ///< Открытое устройство вывода.
  std::unordered_map<SoundType, Voice> voices_;  ///< Свой поток на каждый звук.
};
