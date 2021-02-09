#ifndef VERSION_H
#define VERSION_H
struct version {
	const char *version, *vcs_tag;
};

extern const struct version version;
#endif
