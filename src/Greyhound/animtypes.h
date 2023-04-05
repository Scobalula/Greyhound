#pragma once
#include <cstdint>
typedef unsigned short uint16;

#define STUDIO_LOOPING		0x0001		// ending frame should be the same as the starting frame
#define STUDIO_SNAP			0x0002		// do not interpolate between previous animation and this one
#define STUDIO_DELTA		0x0004		// this sequence "adds" to the base sequences, not slerp blends
#define STUDIO_AUTOPLAY		0x0008		// temporary flag that forces the sequence to always play
#define STUDIO_POST			0x0010		// 
#define STUDIO_ALLZEROS		0x0020		// this animation/sequence has no real animation data
#define STUDIO_FRAMEANIM	0x0040		// animation is encoded as by frame x bone instead of RLE bone x frame
#define STUDIO_CYCLEPOSE	0x0080		// cycle index is taken from a pose parameter index
#define STUDIO_REALTIME		0x0100		// cycle index is taken from a real-time clock, not the animations cycle index
#define STUDIO_LOCAL		0x0200		// sequence has a local context sequence
#define STUDIO_HIDDEN		0x0400		// don't show in default selection views
#define STUDIO_OVERRIDE		0x0800		// a forward declared sequence (empty)
#define STUDIO_ACTIVITY		0x1000		// Has been updated at runtime to activity index
#define STUDIO_EVENT		0x2000		// Has been updated at runtime to event index on server
#define STUDIO_WORLD		0x4000		// sequence blends in worldspace
#define STUDIO_NOFORCELOOP	0x8000	// do not force the animation loop
#define STUDIO_EVENT_CLIENT	0x10000	// Has been updated at runtime to event index on client

// new in respawn models
#define STUDIO_ANIM_UNK			0x20000 // actually first in v52
#define STUDIO_FRAMEMOVEMENT	0x40000
#define STUDIO_ANIM_UNK2		0x80000 // cherry blossom v53, levi in v54

#define NEW_EVENT_STYLE 0x400 // set if string instead of event id, zero otherwise

struct mstudioeventv54_t
{
	float cycle;
	int	event;
	int type; // this will be 0 if old style I'd imagine
	char options[256];

	int szeventindex;
};

// rseq v10
struct mstudioevent54_t_v122
{
	float cycle;
	int	event;
	int type; // this will be 0 if old style I'd imagine

	int unk;

	char options[256]; // this is the only difference compared to normal v54
	int szeventindex;
};


struct mstudioactivitymodifierv53_t
{
	int sznameindex;
	int unk; // 0 or 1 observed.
};

struct mstudioseqdesc_t
{
	int baseptr;

	int	szlabelindex;

	int szactivitynameindex;

	int flags; // looping/non-looping flags

	int activity; // initialized at loadtime to game DLL values
	int actweight;

	int numevents;
	int eventindex;

	Vector3 bbmin; // per sequence bounding box
	Vector3 bbmax;

	int numblends;

	// Index into array of shorts which is groupsize[0] x groupsize[1] in length
	int animindexindex;

	int movementindex; // [blend] float array for blended movement
	int groupsize[2];
	int paramindex[2]; // X, Y, Z, XR, YR, ZR
	Vector2 paramstart; // local (0..1) starting value
	Vector2 paramend; // local (0..1) ending value
	int paramparent;

	float fadeintime; // ideal cross fate in time (0.2 default)
	float fadeouttime; // ideal cross fade out time (0.2 default)

	int localentrynode; // transition node at entry
	int localexitnode; // transition node at exit
	int nodeflags; // transition rules

	float entryphase; // used to match entry gait
	float exitphase; // used to match exit gait

	float lastframe; // frame that should generation EndOfSequence

	int nextseq; // auto advancing sequences
	int pose; // index of delta animation between end and nextseq

	int numikrules;

	int numautolayers;
	int autolayerindex;

	int weightlistindex;

	int posekeyindex;

	int numiklocks;
	int iklockindex;

	// Key values
	int keyvalueindex;
	int keyvaluesize;

	int cycleposeindex; // index of pose parameter to use as cycle index

	int activitymodifierindex;
	int numactivitymodifiers;

	int unk;
	int unk1;

	int unkindex;

	int unk2;
};

struct mstudioseqdesc_t_v16
{
	short szlabelindex;

	short szactivitynameindex;

	int flags; // looping/non-looping flags

	short activity; // initialized at loadtime to game DLL values
	short actweight;

	short numevents;
	short eventindex;

	Vector3 bbmin; // per sequence bounding box
	Vector3 bbmax;

	short numblends;

	// Index into array of shorts which is groupsize[0] x groupsize[1] in length
	short animindexindex;

	//short movementindex; // [blend] float array for blended movement
	short paramindex[2]; // X, Y, Z, XR, YR, ZR
	float paramstart[2]; // local (0..1) starting value
	float paramend[2]; // local (0..1) ending value
	//short paramparent;

	float fadeintime; // ideal cross fate in time (0.2 default)
	float fadeouttime; // ideal cross fade out time (0.2 default)

	byte fill[4];

	// stuff is different after this
	/*
	short localentrynode; // transition node at entry
	short localexitnode; // transition node at exit
	short nodeflags; // transition rules

	float entryphase; // used to match entry gait
	float exitphase; // used to match exit gait

	float lastframe; // frame that should generation EndOfSequence

	short nextseq; // auto advancing sequences
	short pose; // index of delta animation between end and nextseq*/

	short numikrules;

	short numautolayers;
	uint16 autolayerindex;

	uint16 weightlistindex;

	byte groupsize[2];

	uint16 posekeyindex;

	short numiklocks;
	short iklockindex;

	// Key values
	uint16 keyvalueindex;
	short keyvaluesize;

	//short cycleposeindex; // index of pose parameter to use as cycle index

	short activitymodifierindex;
	short numactivitymodifiers;

	int unk;
	int unk1;

	uint16 unkindex;
	short unkcount;
};


// rseq v7.1
struct mstudioanimdesc_t_mod
{
	int baseptr;

	int sznameindex;

	float fps; // frames per second
	int flags; // looping/non-looping flags

	int numframes;

	// piecewise movement
	int nummovements;
	int movementindex;

	int compressedikerrorindex;
	int animindex; // non-zero when anim data isn't in sections

	int numikrules;
	int ikruleindex; // non-zero when IK data is stored in the mdl

	int sectionindex;

	int unk; // what, obviously section related as it's wedged between sectionindex and sectiom frames

	int sectionframes; // number of frames used in each fast lookup section, zero if not used

	int unk1[4];

	// for data transfer
	int mediancount;
	uint64_t somedataoffset;

	//int unk1; // Padding
	//int unk2; // Padding
	//int unk3; // SomeDataOffset
	//int unk4; // SomeDataOffset

	// it seems like there's another int here but I'm unsure
};


// this struct is the wrong size
struct mstudioanimdescv54_t
{
	int baseptr;

	int sznameindex;

	float fps; // frames per second
	int flags; // looping/non-looping flags

	int numframes;

	// piecewise movement
	int nummovements;
	int movementindex;

	int compressedikerrorindex;
	int animindex; // non-zero when anim data isn't in sections

	int numikrules;
	int ikruleindex; // non-zero when IK data is stored in the mdl

	int sectionindex;
	int sectionframes; // number of frames used in each fast lookup section, zero if not used
	int mediancount;
	uint64_t padding;
	uint64_t somedataoffset;
};


// rseq v7.1
struct mstudioanimdescv54_t_v121
{
	int baseptr;

	int sznameindex;

	float fps; // frames per second
	int flags; // looping/non-looping flags

	int numframes;

	// piecewise movement
	int nummovements;
	int movementindex;

	int compressedikerrorindex;
	int animindex; // non-zero when anim data isn't in sections

	int numikrules;
	int ikruleindex; // non-zero when IK data is stored in the mdl

	int sectionindex;

	int unk; // what, obviously section related as it's wedged between sectionindex and sectiom frames

	int sectionframes; // number of frames used in each fast lookup section, zero if not used

	int unk1[4];

	//int unk1; // Padding
	//int unk2; // Padding
	//int unk3; // SomeDataOffset
	//int unk4; // SomeDataOffset

	// it seems like there's another int here but I'm unsure
};

struct mstudioanimdesc_t_v16
{
	float fps; // frames per second	
	int flags; // looping/non-looping flags

	short numframes;

	// piecewise movement
	//short nummovements;
	//short movementindex;

	short unk_v16;

	uint16 sznameindex;

	uint16 compressedikerrorindex;

	int animindex; // non-zero when anim data isn't in sections

	short numikrules;
	uint16 ikruleindex; // non-zero when IK data is stored in the mdl

	uint16 unk1[5];

	uint16 sectionindex;

	uint16 unk2; // what, obviously section related as it's wedged between sectionindex and sectiom frames

	uint16 sectionframes; // number of frames used in each fast lookup section, zero if not used
};


// rle anim flags
#define STUDIO_ANIM_SCALE 1
#define STUDIO_ANIM_ROT 2
#define STUDIO_ANIM_POS 4

// bone anim flags
#define STUDIO_ANIM_BONEPOS 1
#define STUDIO_ANIM_BONEROT 2
#define STUDIO_ANIM_BONESCALE 4

struct mstudio_rle_anim_t
{
	uint16_t size : 12;
	uint16_t bAdditiveCustom : 1;
	uint16_t bAnimScale : 1;			// If zero, one per data set
	uint16_t bAnimRotation : 1;		// If zero, one per data set
	uint16_t bAnimPosition : 1;	// If zero, one per data set
};

struct mstudioanim_valueptr_t
{
	short offset : 13;
	short flags : 3;
	uint8_t axisIdx1;
	uint8_t axisIdx2;
};

union mstudioanimvalue_t
{
	struct {
		byte valid;
		byte total;
	} num;
	short value;
};