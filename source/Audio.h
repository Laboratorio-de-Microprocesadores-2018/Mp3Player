/**
 * @file Audio.h
 * @brief Interface between the Mp3Player and the hardware which outputs audio.
 *
 * This module implements the interface to stream audio samples from decoder
 * to hardware. The API is independant from the audio output chosen (DAC, PWM, DAC).
 * Audio is sent in frames, and a queue of them is stores to have a continuous stream
 * without interrupts.
 */
#ifndef AUDIO_H_
#define AUDIO_H_

/////////////////////////////////////////////////////////////////////////////////
//                             Included header files                           //
/////////////////////////////////////////////////////////////////////////////////
#include "fsl_common.h"

/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////
#define DAC_OUTPUT 0
#define I2S_OUTPUT 1

#define AUDIO_OUTPUT DAC_OUTPUT

/////////////////////////////////////////////////////////////////////////////////
//                         				API	  		                           //
/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Initialize Audio module, pins and peripherals.
 *
 */
status_t Audio_Init();

/**
 * @brief Deinitialize Audio module, pins and peripherals.
 *
 * This function must be called before power down to configure all peripherals in
 * static mode.
 */
status_t Audio_Deinit();


/**
 * @brief Start streaming sound samples.
 *
 * Subsequent calls to Audio_PushFrame() will add frames to the queue.
 *
 */
void Audio_Play();

/**
 * @brief Pause the audio streaming.
 * The content of the streaming queue is preserved.
 */
void Audio_Pause();

/**
 * @brief Resume the audio streaming.
 * To be called after Audio_Pause().
 */
void Audio_Resume();

/**
 * @brief Stop the audio streaming.
 * The content of the streaming queue is deleted.
 */
void Audio_Stop();

/**
 * @brief DEBERIA SER ESTATICA??
 *
 */
void Audio_SetSampleRate(uint32_t sr);

/**
 * @brief Push a frame to the queue.
 *
 *
 * @param samples Pointer to samples array.
 * @param nSamples Number of samples contained in the buffer.
 * @param sampleRate Sample rate of the frame.
 * @param frameNumber Number of the frame.
 * @return True if there is still place in the queue, false otherwise.
 */
void Audio_PushFrame(int16_t* samples, uint16_t nSamples, uint32_t sampleRate, uint32_t frameNumber);

/**
 * @brief
 * @TODO Maybe its unnecessary, consider to delete it.
 *
 */
uint16_t * Audio_GetBackBuffer();

/**
 * @brief Checks if queue is not full.
 * @return True if at least one audio frame in the queue is empty.
 */
bool Audio_QueueIsFree();

/**
 * @brief Check if queue is empty.
 * @return True if the queue has no audio frames left to stream.
 */
bool Audio_QueueIsEmpty();

/**
 * @brief Reset the audio frames queue.
 *
 */
void Audio_ResetQueue();

/**
 * @brief Return the number of the frame currently playing.
 * Due to the queue present in this module, there is a time delay between the
 * moment a frame is pushed and the moment it is played. This function is useful
 * to synchronize audio stream to graphical data (e.g. fft plot).
 */
uint32_t Audio_GetCurrentFrameNumber();


#endif /* AUDIO_H_ */
