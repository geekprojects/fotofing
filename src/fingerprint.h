#ifndef __FOTOFING_FINGERPRINT_H_
#define __FOTOFING_FINGERPRINT_H_

#include <string>

// How many blocks to divide images up in to. Changing this will
// result in completely different fingerprints
#define FINGERPRINT_DIVIDE 12

std::string fingerprint(std::string path);

#endif
