#ifndef MIGHTEX1304_h
#define MIGHTEX1304_h
/**
 * @file mightex1304.h
 * @author Paolo Bosetti (paolo.bosetti@unitn.it)
 * @brief Userland driver for Mightex TCE-1304-U line CCD camera
 * @date 2021-06-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdlib.h>
#ifdef _WIN32
#include <stdint.h>
#include <Windows.h>
#define DLLEXPORT __declspec( dllexport )
#else
#define DLLEXPORT
#endif // _WIN32
#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Number of standard pixels
 */
#define MTX_PIXELS 3648

/**
 * @brief Number of "light-shield" pixels
 * 
 * Those are pixels that are shielded from light: their output provides a 
 * measure of the dark current in the sensor. Their values shall be averaged
 * and subtracted from measure values
 * 
 * @see mightex_dark_mean and mightex_read_frame
 */
#define MTX_DARK_PIXELS 13

typedef unsigned char BYTE;

/**
 * @brief The two possible operating modes: continuous or triggered
 */
typedef enum { MTX_NORMAL_MODE = 0, MTX_TRIGGER_MODE = 1 } mtx_mode_t;

/**
 * @brief Standard exit values for library functions
 */
typedef enum { MTX_FAIL = 0, MTX_OK = 1 } mtx_result_t;

#ifndef SWIG

/**
 * @brief Opaque structure encapsulating the driver
 */
typedef struct mightex mightex_t;

/**
 * @brief Create a new Mightex object
 * 
 * @return mightex_t* 
 */
DLLEXPORT
mightex_t *mightex_new();

/**
 * @brief Set exposure time, in milliseconds
 * 
 * @param m the Mightex object
 * @param t the exposure time, in ms
 * @return mtx_result_t 
 */
DLLEXPORT
mtx_result_t mightex_set_exptime(mightex_t *m, float t);

/**
 * @brief Return the number of available frames
 * 
 * The internal buffer of the Mightex 1304 can hold a maximum of 4 frames,
 * so this function returns a value from 0 to 4. Negative values mean an
 * error in the underlying usb driver.
 * 
 * @param m the Mightex object
 * @return int 
 */
DLLEXPORT
int mightex_get_buffer_count(mightex_t *m);

/**
 * @brief Read a frame from the camera buffer
 * 
 * Read a frame and store it internally. Frame data can be accessed with the 
 * proper accessors. In particular, the pixel values array is stored in the 
 * location returned by @ref mightex_frame_p. Timestamp and dark mean are also 
 * updated.
 * 
 * @param m the Mightex object
 * @return mtx_result_t 
 * @see mightex_frame_p, mightex_frame_timestamp, mightex_dark_mean
 */
DLLEXPORT
mtx_result_t mightex_read_frame(mightex_t *m);


/**
 * @brief Close the object
 * 
 * Close the object connection and free all resources.
 * 
 * @param m the Mightex object
 */
DLLEXPORT
void mightex_close(mightex_t *m);

/**
 * @brief Set the operating mode
 * 
 * @param m the Mightex object
 * @param mode the desired mode
 * @return mtx_result_t 
 * @see mtx_mode_t
 */
DLLEXPORT
mtx_result_t mightex_set_mode(mightex_t *m, mtx_mode_t mode);

/**
 * @brief GPIO register write
 * 
 * Write a given value to a GPIO register.
 * 
 * @param m 
 * @param reg register number (0--3)
 * @param val either 0 or 1
 */
DLLEXPORT
void mightex_gpio_write(mightex_t *m, BYTE reg, BYTE val);

/**
 * @brief GPIO register read
 * 
 * Read the current value of GPIO register.
 * 
 * @param m 
 * @param reg register number (0--3)
 * @return BYTE the current GPIO level (0 or 1)
 */
DLLEXPORT
BYTE mightex_gpio_read(mightex_t *m, BYTE reg);


/** @name Filters and Estimators
 * 
 * These functions allow to automatically apply filters and estimators on a
 * frame.
 * 
 * A *filter* is a function that operates in place on all the individual pixel
 * values. Useful for scaling, dark value removal, thresholding, etc.
 * 
 * An *estimator* is a function that operates on all pixel values and returns
 * a single estimate (a mean value, peak, etc.). 
 */
 /**@{*/ 

/**
 * @brief Filter prototype
 * 
 * This is a filter function that operates on all elements in @ref 
 * mightex_frame_p in place. Ideally, @ref mightex_raw_frame_p shall remain 
 * unchanged and always hold original raw data, while @ref mightex_frame_p is
 * initially (i.e. just after calling @ref mightex_read_frame) a copy of raw 
 * data, then this function is applied by calling @ref mightex_apply_filter.
 * 
 * @param m 
 * @param raw_data an array of data (the raw values from @ref 
 * mightex_raw_frame_p)
 * @param len the array length
 * @param ud optional user data (can be NULL)
 * @return typedef 
 */
typedef void mightex_filter_t(mightex_t *m, uint16_t * const raw_data, uint16_t len, void *ud);

/**
 * @brief Estimator prototype
 * 
 * @param m 
 * @param data an array of data (the raw values from @ref mightex_raw_frame_p)
 * @param len the aray length
 * @param ud optional user data (can be NULL)
 * @return typedef 
 */
typedef double mightex_estimator_t(mightex_t *m, uint16_t * const data, uint16_t len, void *ud);

/**
 * @brief Set the filter function
 * 
 * By default, the filter function subtracts the dark pixels average. Set it
 * to NULL if you want to disable this behavior.
 * 
 * @param m 
 * @param filter the function to be used when calling @ref mightex_apply_filter
 */
DLLEXPORT
void mightex_set_filter(mightex_t *m, mightex_filter_t *filter);

/**
 * @brief Reset the filter to the default one
 * 
 * By default, the filter function subtracts the dark pixels average. Useful
 * when you changed it with @ref mightex_set_filter and later on you want to
 * reset it to the default behavior.
 * 
 * @param m 
 */
DLLEXPORT
void mightex_reset_filter(mightex_t *m);

/**
 * @brief Apply the filter function set with @ref mightex_set_filter
 * 
 * By default, the filter function subtracts the dark pixels average.
 * 
 * @param m 
 * @param userdata a user provided data (typically a struct pointer)
 */
DLLEXPORT
void mightex_apply_filter(mightex_t *m, void *userdata);

/**
 * @brief Set the estimator function
 * 
 * @param m 
 * @param estimator 
 * @note the estimator function works on the **filtered** frame data
 */
DLLEXPORT
void mightex_set_estimator(mightex_t *m, mightex_estimator_t *estimator);

/**
 * @brief Reset the estimator to the default one
 * 
 * By default, the estimators caculates the weighted average of the filtered
 * image data, thresholding the data to a level equal to three times the dark 
 * level.
 * 
 * @param m 
 */
DLLEXPORT
void mightex_reset_estimator(mightex_t *m);

/**
 * @brief Apply the estimator function
 * 
 * @param m 
 * @param userdata 
 * @return double 
 */
DLLEXPORT
double mightex_apply_estimator(mightex_t *m, void *userdata);

/**@}*/

/** @name Accessors
 * 
 * Accessors to Mightex object parameters
 */
/**@{*/

/**
 * @brief The serial number of the connected device
 * 
 * This is useful whenever you have two or more cameras of the same model 
 * connected to the same host.
 * 
 * @param m 
 * @return char* a pointer to the serial number string (internally stored)
 */
DLLEXPORT
char *mightex_serial_no(mightex_t *m);

/**
 * @brief The firmware version of the connected device
 * 
 * Returns an internally allocated string.
 * 
 * @param m 
 * @return char* a pointer to the firmware version string (internally stored)
 */
DLLEXPORT
char *mightex_version(mightex_t *m);

/**
 * @brief The Mightex library software version and details
 * 
 * Returns an internally allocated string.
 * 
 * @param m 
 * @return char* char* a pointer to the library version string (internally
 *  stored)
 */
DLLEXPORT
char *mightex_sw_version();

/**
 * @brief Return the pointer to the raw image storage area
 * 
 * The last frame, as collected with @ref mightex_read_frame, is stored as an
 * array of `uint16_t` in the location pointed by the returned pointer.
 * 
 * @param m 
 * @return uint16_t* An array of @ref MTX_PIXELS elements
 */
DLLEXPORT
uint16_t *mightex_frame_p(mightex_t *m);

/**
 * @brief Return the pointer to the filtered image storage area
 * 
 * The last frame, as collected with @ref mightex_read_frame, is copied as an
 * array of `uint16_t` in the location pointed by the returned pointer. This
 * array of data is filtered upon calling @ref mightex_apply_filter.
 * 
 * @param m 
 * @return uint16_t* An array of @ref MTX_PIXELS elements
 */
DLLEXPORT
uint16_t *mightex_raw_frame_p(mightex_t *m);


/**
 * @brief The timestamp of the last grabbed frame
 * 
 * @param m 
 * @return uint16_t Timestamp of the last grabbed frame
 * @note The values **are not** compensated for the dark current average!
 */
DLLEXPORT
uint16_t mightex_frame_timestamp(mightex_t *m);

/**
 * @brief The mean of the shielded pixels
 * 
 * This returns the averaged value of the @ref MTX_DARK_PIXELS. This value
 * gives an estimate of the sensor dark current.
 * 
 * @param m 
 * @return uint16_t The average dark current value
 */
DLLEXPORT
uint16_t mightex_dark_mean(mightex_t *m);

/**
 * @brief Return the number of pixels (@ref MTX_PIXELS)
 * 
 * @param m 
 * @return uint16_t 
 */
DLLEXPORT
uint16_t mightex_pixel_count(mightex_t *m);

/**
 * @brief Return the number of shielded pixels (@ref MTX_DARK_PIXELS)
 * 
 * @param m 
 * @return uint16_t 
 */
DLLEXPORT
uint16_t mightex_dark_pixel_count(mightex_t *m);
/**@}*/

#endif //SWIG

#ifdef __cplusplus
}
#endif

#endif // double inclusion guard