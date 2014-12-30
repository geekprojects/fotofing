#ifndef __FOTOFING_EXIFTAGS_H_
#define __FOTOFING_EXIFTAGS_H_

#define EXIF_Image "Image"
#define EXIF_Image_Make                 EXIF_Image, 0x010f
#define EXIF_Image_Model                EXIF_Image, 0x0110
#define EXIF_Image_DateTime             EXIF_Image, 0x0132
#define EXIF_Image_DateTimeOriginal     EXIF_Image, 0x9003

#define EXIF_Photo "Photo"
#define EXIF_Photo_BodySerialNumber     EXIF_Photo, 0xa431
#define EXIF_Photo_LensMake             EXIF_Photo, 0xa433
#define EXIF_Photo_LensModel            EXIF_Photo, 0xa434

#define EXIF_Canon "Canon"
#define EXIF_Canon_SerialNumber         EXIF_Canon, 0x000c
#define EXIF_Canon_InternalSerialNumber EXIF_Canon, 0x0096

#define EXIF_Panasonic "Panasonic"
#define EXIF_Panasonic_InternalSerialNumber EXIF_Panasonic, 0x0025

#endif
