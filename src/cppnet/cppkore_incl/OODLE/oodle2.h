
//===================================================
// Oodle2 Core header
// (C) Copyright 1994-2021 Epic Games Tools LLC
//===================================================

#ifndef __OODLE2_H_INCLUDED__
#define __OODLE2_H_INCLUDED__

#ifndef OODLE2_PUBLIC_HEADER
#define OODLE2_PUBLIC_HEADER 1
#endif

#ifndef __OODLE2BASE_H_INCLUDED__
#include "oodle2base.h"
#endif

#ifdef _MSC_VER
#pragma pack(push, Oodle, 8)

#pragma warning(push)
#pragma warning(disable : 4127) // conditional is constant
#endif

// header version :
//  the DLL is incompatible when MAJOR is bumped
//  MINOR is for internal revs and bug fixes that don't affect API compatibility
#define OODLE2_VERSION_MAJOR            9
#define OODLE2_VERSION_MINOR            0

// OodleVersion string is 1 . MAJOR . MINOR
//  don't make it from macros cuz the doc tool has to parse the string literal

#define OodleVersion "2.9.0"    /*
*/

//-----------------------------------------------------
// OodleLZ

#if 0
#define OODLE_ALLOW_DEPRECATED_COMPRESSORS /* If you need to encode with the deprecated compressors, define this before including oodle2.h

    You may still decode with them without defining this.
*/
#endif

// Default verbosity selection of 0 will not even log when it sees corruption
typedef enum OodleLZ_Verbosity
{
    OodleLZ_Verbosity_None = 0,
    OodleLZ_Verbosity_Minimal = 1,
    OodleLZ_Verbosity_Some = 2,
    OodleLZ_Verbosity_Lots = 3,
    OodleLZ_Verbosity_Force32 = 0x40000000
} OodleLZ_Verbosity;
/* Verbosity of LZ functions
    LZ functions print information to the function set by $OodleCore_Plugins_SetPrintf
    or $OodleXLog_Printf if using OodleX.
*/

OO_COMPILER_ASSERT( sizeof(OodleLZ_Verbosity) == 4 );

typedef enum OodleLZ_Compressor
{
    OodleLZ_Compressor_Invalid = -1,
    OodleLZ_Compressor_None = 3,  // None = memcpy, pass through uncompressed bytes

    // NEW COMPRESSORS :
    OodleLZ_Compressor_Kraken = 8,    // Fast decompression and high compression ratios, amazing!
    OodleLZ_Compressor_Leviathan = 13,// Leviathan = Kraken's big brother with higher compression, slightly slower decompression.
    OodleLZ_Compressor_Mermaid = 9,   // Mermaid is between Kraken & Selkie - crazy fast, still decent compression.
    OodleLZ_Compressor_Selkie = 11,   // Selkie is a super-fast relative of Mermaid.  For maximum decode speed.
    OodleLZ_Compressor_Hydra = 12,    // Hydra, the many-headed beast = Leviathan, Kraken, Mermaid, or Selkie (see $OodleLZ_About_Hydra)

#ifdef OODLE_ALLOW_DEPRECATED_COMPRESSORS
    OodleLZ_Compressor_BitKnit = 10, // no longer supported as of Oodle 2.9.0
    OodleLZ_Compressor_LZB16 = 4, // DEPRECATED but still supported
    OodleLZ_Compressor_LZNA = 7,  // no longer supported as of Oodle 2.9.0
    OodleLZ_Compressor_LZH = 0,   // no longer supported as of Oodle 2.9.0
    OodleLZ_Compressor_LZHLW = 1, // no longer supported as of Oodle 2.9.0
    OodleLZ_Compressor_LZNIB = 2, // no longer supported as of Oodle 2.9.0
    OodleLZ_Compressor_LZBLW = 5, // no longer supported as of Oodle 2.9.0
    OodleLZ_Compressor_LZA = 6,   // no longer supported as of Oodle 2.9.0
#endif

    OodleLZ_Compressor_Count = 14,
    OodleLZ_Compressor_Force32 = 0x40000000
} OodleLZ_Compressor;
/* Selection of compression algorithm.

    Each compressor provides a different balance of speed vs compression ratio.

    New Oodle users should only use the new sea monster family of compressors.

    The OODLE_ALLOW_DEPRECATED_COMPRESSORS set of compressors is no longer supported
    as of Oodle 2.9.0 ; see $Oodle_FAQ_deprecated_compressors

    The sea monsters are all fuzz safe and use whole-block quantum (not the 16k quantum)
    ($OodleLZ_Compressor_UsesWholeBlockQuantum)

    If you need to encode the deprecated compressors, define $OODLE_ALLOW_DEPRECATED_COMPRESSORS before
    including oodle2.h  

    See $Oodle_FAQ_WhichLZ for a quick FAQ on which compressor to use

    See $OodleLZ_About for discussion of how to choose a compressor.
*/

OO_COMPILER_ASSERT( sizeof(OodleLZ_Compressor) == 4 );

typedef enum OodleLZ_PackedRawOverlap
{
    OodleLZ_PackedRawOverlap_No = 0,
    OodleLZ_PackedRawOverlap_Yes = 1,
    OodleLZ_PackedRawOverlap_Force32 = 0x40000000
} OodleLZ_PackedRawOverlap;
/* Bool enum
*/

typedef enum OodleLZ_CheckCRC
{
    OodleLZ_CheckCRC_No = 0,
    OodleLZ_CheckCRC_Yes = 1,
    OodleLZ_CheckCRC_Force32 = 0x40000000
} OodleLZ_CheckCRC;
/* Bool enum for the LZ decoder - should it check CRC before decoding or not?

    NOTE : the CRC's in the LZH decompress checks are the CRC's of the *compressed* bytes.  This allows checking the CRc
    prior to decompression, so corrupted data cannot be fed to the compressor.

    To use OodleLZ_CheckCRC_Yes, the compressed data must have been made with $(OodleLZ_CompressOptions:sendQuantumCRCs) set to true.

    If you want a CRC of the raw bytes, there is one optionally stored in the $OodleLZ_SeekTable and can be confirmed with
    $OodleLZ_CheckSeekTableCRCs
*/


typedef enum OodleLZ_Profile
{
    OodleLZ_Profile_Main=0,         // Main profile (all current features allowed)
    OodleLZ_Profile_Reduced=1,      // Reduced profile (Kraken only, limited feature set)
    OodleLZ_Profile_Force32 = 0x40000000
} OodleLZ_Profile;
/* Decode profile to target */

// Not flagged for idoc and done using a #define since it's internal (testing) use only
#define OodleLZ_Profile_Internal_Custom ((OodleLZ_Profile)100)

OO_COMPILER_ASSERT( sizeof(OodleLZ_Profile) == 4 );

typedef enum OodleDecompressCallbackRet
{
    OodleDecompressCallbackRet_Continue=0,
    OodleDecompressCallbackRet_Cancel=1,
    OodleDecompressCallbackRet_Invalid=2,
    OodleDecompressCallbackRet_Force32 = 0x40000000
} OodleDecompressCallbackRet;
/* Return value for $OodleDecompressCallback
    return OodleDecompressCallbackRet_Cancel to abort the in-progress decompression
*/

OODEFFUNC typedef OodleDecompressCallbackRet (OODLE_CALLBACK OodleDecompressCallback)(void * userdata, const OO_U8 * rawBuf,OO_SINTa rawLen,const OO_U8 * compBuf,OO_SINTa compBufferSize , OO_SINTa rawDone, OO_SINTa compUsed);
/* User-provided callback for decompression

    $:userdata  the data you passed for _pcbData_
    $:rawBuf    the decompressed buffer
    $:rawLen    the total decompressed length
    $:compBuf   the compressed buffer
    $:compBufferSize  the total compressed length
    $:rawDone   number of bytes in rawBuf decompressed so far
    $:compUsed  number of bytes in compBuf consumed so far

    OodleDecompressCallback is called incrementally during decompression.
*/

typedef enum OodleLZ_CompressionLevel
{
    OodleLZ_CompressionLevel_None=0,        // don't compress, just copy raw bytes
    OodleLZ_CompressionLevel_SuperFast=1,   // super fast mode, lower compression ratio
    OodleLZ_CompressionLevel_VeryFast=2,    // fastest LZ mode with still decent compression ratio
    OodleLZ_CompressionLevel_Fast=3,        // fast - good for daily use
    OodleLZ_CompressionLevel_Normal=4,      // standard medium speed LZ mode

    OodleLZ_CompressionLevel_Optimal1=5,    // optimal parse level 1 (faster optimal encoder)
    OodleLZ_CompressionLevel_Optimal2=6,    // optimal parse level 2 (recommended baseline optimal encoder)
    OodleLZ_CompressionLevel_Optimal3=7,    // optimal parse level 3 (slower optimal encoder)
    OodleLZ_CompressionLevel_Optimal4=8,    // optimal parse level 4 (very slow optimal encoder)
    OodleLZ_CompressionLevel_Optimal5=9,    // optimal parse level 5 (don't care about encode speed, maximum compression)

    OodleLZ_CompressionLevel_HyperFast1=-1, // faster than SuperFast, less compression
    OodleLZ_CompressionLevel_HyperFast2=-2, // faster than HyperFast1, less compression
    OodleLZ_CompressionLevel_HyperFast3=-3, // faster than HyperFast2, less compression
    OodleLZ_CompressionLevel_HyperFast4=-4, // fastest, less compression

    // aliases :
    OodleLZ_CompressionLevel_HyperFast=OodleLZ_CompressionLevel_HyperFast1, // alias hyperfast base level
    OodleLZ_CompressionLevel_Optimal = OodleLZ_CompressionLevel_Optimal2,   // alias optimal standard level
    OodleLZ_CompressionLevel_Max     = OodleLZ_CompressionLevel_Optimal5,   // maximum compression level
    OodleLZ_CompressionLevel_Min     = OodleLZ_CompressionLevel_HyperFast4, // fastest compression level

    OodleLZ_CompressionLevel_Force32 = 0x40000000,
    OodleLZ_CompressionLevel_Invalid = OodleLZ_CompressionLevel_Force32
} OodleLZ_CompressionLevel;
/* Selection of compression encoder complexity

    Higher numerical value of CompressionLevel = slower compression, but smaller compressed data.

    The compressed stream is always decodable with the same decompressors.
    CompressionLevel controls the amount of work the encoder does to find the best compressed bit stream.
    CompressionLevel does not primary affect decode speed, it trades off encode speed for compressed bit stream quality.

    I recommend starting with OodleLZ_CompressionLevel_Normal, then try up or down if you want
    faster encoding or smaller output files.

    The Optimal levels are good for distribution when you compress rarely and decompress often;
    they provide very high compression ratios but are slow to encode.  Optimal2 is the recommended level
    to start with of the optimal levels.
    Optimal4 and 5 are not recommended for common use, they are very slow and provide the maximum compression ratio,
    but the gain over Optimal3 is usually small.

    The HyperFast levels have negative numeric CompressionLevel values.
    They are faster than SuperFast for when you're encoder CPU time constrained or want
    something closer to symmetric compression vs. decompression time.
    The HyperFast levels are currently only available in Kraken, Mermaid & Selkie.
    Higher levels of HyperFast are faster to encode, eg. HyperFast4 is the fastest.

    The CompressionLevel does not affect decode speed much.  Higher compression level does not mean
    slower to decode.  To trade off decode speed vs ratio, use _spaceSpeedTradeoffBytes_ in $OodleLZ_CompressOptions

*/

OO_COMPILER_ASSERT( sizeof(OodleLZ_CompressionLevel) == 4 );

typedef enum OodleLZ_Jobify
{
    OodleLZ_Jobify_Default=0,       // Use compressor default for level of internal job usage
    OodleLZ_Jobify_Disable=1,       // Don't use jobs at all
    OodleLZ_Jobify_Normal=2,        // Try to balance parallelism with increased memory usage
    OodleLZ_Jobify_Aggressive=3,    // Maximize parallelism even when doing so requires large amounts of memory
    OodleLZ_Jobify_Count=4,

    OodleLZ_Jobify_Force32 = 0x40000000,
} OodleLZ_Jobify;
/* Controls the amount of internal threading in $OodleLZ_Compress calls

    Once you install a pluggable job system via $OodleCore_Plugins_SetJobSystem, Oodle can internally break
    heavy-weight compression tasks into smaller jobs that can run in parallel.  This can speed up
    compression of large blocks of data at Optimal1 and higher levels substantially.

    The trade-off is that running more jobs concurrently rather than sequentially can greatly increase
    memory requirements when there are multiple outstanding memory-intensive jobs.

    OodleLZ_Jobify_Default lets the compressor decide; typically compressors will default to "Normal"
    when a pluggable job system has been installed, and "Disable" otherwise.

    OodleLZ_Jobify_Disable disables use of internal jobs entirely; all compression work is done on
    the calling thread.  This minimizes the amount of memory used, and is also appropriate when you're
    getting parallelism in other ways, e.g. by running OodleLZ_Compress on many threads yourself.

    OodleLZ_Jobify_Normal uses jobs to increase compressor parallelism and speeds up compression of
    large blocks of data, but avoids handing out many concurrent jobs for tasks that are memory-intensive.

    OodleLZ_Jobify_Aggressive will use concurrent jobs even for highly memory-intensive tasks.  This
    can speed up things further, but at a potentially significant increase in the amount of memory used
    by Oodle.

*/

#define OODLELZ_LOCALDICTIONARYSIZE_MAX     (1<<30) /* Maximum value of maxLocalDictionarySize in OodleLZ_CompressOptions
*/

#define OODLELZ_SPACESPEEDTRADEOFFBYTES_DEFAULT (256) /* Default value of spaceSpeedTradeoffBytes in OodleLZ_CompressOptions
    Changes how the encoder makes decisions in the bit stream
    Higher spaceSpeedTradeoffBytes favors decode speed more (larger compressed files)
    Lower spaceSpeedTradeoffBytes favors smaller compressed files (slower decoder)
    Goes in a power of 2 scale; so try 64,128 and 512,1024
    (OODLELZ_SPACESPEEDTRADEOFFBYTES_DEFAULT/2) or (OODLELZ_SPACESPEEDTRADEOFFBYTES_DEFAULT*2)
*/


typedef OOSTRUCT OodleLZ_CompressOptions
{
    OO_U32              unused_was_verbosity;               // unused ; was verbosity (set to zero)
    OO_S32              minMatchLen;        // minimum match length ; cannot be used to reduce a compressor's default MML, but can be higher.  On some types of data, a large MML (6 or 8) is a space-speed win.
    OO_BOOL             seekChunkReset;     // whether chunks should be independent, for seeking and parallelism
    OO_S32              seekChunkLen;       // length of independent seek chunks (if seekChunkReset) ; must be a power of 2 and >= $OODLELZ_BLOCK_LEN ; you can use $OodleLZ_MakeSeekChunkLen
    OodleLZ_Profile     profile;            // decoder profile to target (set to zero)
    OO_S32              dictionarySize;     // sets a maximum offset for matches, if lower than the maximum the format supports.  <= 0 means infinite (use whole buffer).  Often power of 2 but doesn't have to be.
    OO_S32              spaceSpeedTradeoffBytes;  // this is a number of bytes; I must gain at least this many bytes of compressed size to accept a speed-decreasing decision
    OO_S32              unused_was_maxHuffmansPerChunk;  //  unused ; was maxHuffmansPerChunk
    OO_BOOL             sendQuantumCRCs;    // should the encoder send a CRC of each compressed quantum, for integrity checks; this is necessary if you want to use OodleLZ_CheckCRC_Yes on decode
    OO_S32              maxLocalDictionarySize;  // (Optimals) size of local dictionary before needing a long range matcher.  This does not set a window size for the decoder; it's useful to limit memory use and time taken in the encoder.  maxLocalDictionarySize must be a power of 2.  Must be <= OODLELZ_LOCALDICTIONARYSIZE_MAX
    OO_BOOL             makeLongRangeMatcher;   // (Optimals) should the encoder find matches beyond maxLocalDictionarySize using an LRM
    OO_S32              matchTableSizeLog2; //(non-Optimals)  when variable, sets the size  of the match finder structure (often a hash table) ; use 0 for the compressor's default

    OodleLZ_Jobify      jobify;         // controls internal job usage by compressors
    void *              jobifyUserPtr;  // user pointer passed through to RunJob and WaitJob callbacks

    OO_S32              farMatchMinLen; // far matches must be at least this len
    OO_S32              farMatchOffsetLog2; // if not zero, the log2 of an offset that must meet farMatchMinLen

    OO_U32              reserved[4];   // reserved space for adding more options; zero these!
} OodleLZ_CompressOptions;
/* Options for the compressor

    Typically filled by calling $OodleLZ_CompressOptions_GetDefault , then individual options may be modified, like :

    OodleLZ_CompressOptions my_options = *OodleLZ_CompressOptions_GetDefault()

    To ensure you have set up the options correctly, call $OodleLZ_CompressOptions_Validate.

    _unused_was_verbosity_ : place holder, set to zero

    _minMatchLen_ : rarely useful.  Default value of 0 means let the compressor decide.  On some types of data,
    bumping this up to 4,6, or 8 can improve decode speed with little effect on compression ratio.  Most of the
    Oodle compressors use a default MML of 4 at levels below 7, and MML 3 at levels >= 7.  If you want to keep MML 4
    at the higher levels, set _minMatchLen_ here to 4.  _minMatchLen_ cannot be used to reduce the base MML of the compressor, only to increase it.

    _seekChunkReset_ must be true if you want the decode to be able to run "Wide", with pieces that can be
    decoded independently (not keeping previous pieces in memory for match references).

    _seekChunkLen_ : length of independent seek chunks (if seekChunkReset) ; must be a power of 2 and >= $OODLELZ_BLOCK_LEN ; you can use $OodleLZ_MakeSeekChunkLen

    _profile_ : tells the encoder to target alternate bitstream profile.  Default value of zero for normal use.

    _dictionarySize_ : limits the encoder to partial buffer access for matches.  Can be useful for decoding incrementally
    without keeping the entire output buffer in memory.

    _spaceSpeedTradeoffBytes_ is a way to trade off compression ratio for decode speed.  If you make it smaller,
    you get more compression ratio and slower decodes.  It's the number of bytes that a decision must save to
    be worth a slower decode.  Default is 256 (OODLELZ_SPACESPEEDTRADEOFFBYTES_DEFAULT).  So that means the encoder must be able to save >= 256 bytes to
    accept something that will slow down decoding (like adding another Huffman table).  The typical range is
    64-1024.

    Lower _spaceSpeedTradeoffBytes_ = more compression, slower decode
    Higher _spaceSpeedTradeoffBytes_ = less compression, faster decode

    _spaceSpeedTradeoffBytes_ is the primary parameter for controlling Hydra.  The default value of 256 will make
    Hydra decodes that are just a little bit faster than Kraken.  You get Kraken speeds around 200, and Mermaid
    speeds around 1200.

    At the extreme, a _spaceSpeedTradeoffBytes_ of zero would mean all you care about is compression ratio, not decode
    speed, you want the encoder to make the smallest possible output.  (you cannot actually set zero, as zero values
    always mean "use default" in this struct; you never really want zero anyway)
    Generally _spaceSpeedTradeoffBytes_ below 16 provides diminishing gains in size with pointless decode speed loss.

    _spaceSpeedTradeoffBytes_ is on sort of powers of 2 scale, so you might want to experiment with 32,64,128,256,512

    _spaceSpeedTradeoffBytes_ outside the range [16 - 2048] is not recommended.

    _unused_was_maxHuffmansPerChunk_ : place holder, set to zero

    _sendQuantumCRCs_ : send hashes of the compressed data to verify in the decoder; not recommended, if you need data
    verification, use your own system outside of Oodle.  DEPRECATED, not recommended.  For backwards compatibility only.

    _maxLocalDictionarySize_ : only applies to optimal parsers at level >= Optimal2.  This limits the encoder memory use.
    Making it larger = more compression, higher memory use.  Matches within maxLocalDictionarySize are found exactly,
    outside the maxLocalDictionarySize window an approximate long range matcher is used.

    _makeLongRangeMatcher_ : whether an LRM should be used to find matches outside the _maxLocalDictionarySize_ window
    (Optimal levels only)

    _matchTableSizeLog2_ : for non-optimal levels (level <= Normal), controls the hash table size.  Making this very
    small can sometimes boost encoder speed.  For the very fastest encoding, use the SuperFast level and change
    _matchTableSizeLog2_ to 12 or 13.

    _matchTableSizeLog2_ should usually be left zero to use the encoder's default

    _matchTableSizeLog2_ allows you to limit memory use of the non-Optimal encoder levels.  Memory use is roughly
    ( 1 MB + 4 << matchTableSizeLog2 )

    _jobify_ tells compressors how to use internal jobs for compression tasks.  Jobs can be run in parallel using the
    job system plugins set with $OodleCore_Plugins_SetJobSystem.  Not all compressors or compression level support
    jobs, but the slower ones generally do.  The default value of jobify is to use a thread system if one is installed.

    _farMatchMinLen_ and _farMatchOffsetLog2_ can be used to tune the encoded stream for a known cache size on the
    decoding hardware.  If set, then offsets with log2 greater or each to _farMatchOffsetLog2_ must have a minimum
    length of _farMatchMinLen_.  For example to target a machine with a 2 MB cache, set _farMatchOffsetLog2_ to 21,
    and _farMatchMinLen_ to something large, like 16 or 20.

    Without _farMatchMinLen_ and _farMatchOffsetLog2_ set, the Oodle encoders tune for a blend of cache sizes that works
    well on most machines.  _dictionarySize_ can also be used to tune for cache size, but cuts off all matches
    beyond a certain distance.  That may be more appropriate when you don't want to go out of cache at all.
    _farMatchMinLen_ can only be used to make the standard blend target more restrictive; it can reduce the target cache size
    but can't make it larger (or it can raise min match len outside cache but can't make it shorter).

    For help on setting up OodleLZ_CompressOptions contact support at oodle@radgametools.com

    NOTE : fields you do not set should always be zero initialized.  In particular the _reserved_ fields should be zeroed.
    Zero always means "use default" and is a future-portable initialization value.

    If you set fields to zero to mean "use default" you can call $OodleLZ_CompressOptions_Validate to change them
    to default values.  This is done automatically internally if you don't do it explicitly.

*/

typedef enum OodleLZ_Decode_ThreadPhase
{
    OodleLZ_Decode_ThreadPhase1 = 1,
    OodleLZ_Decode_ThreadPhase2 = 2,
    OodleLZ_Decode_ThreadPhaseAll = 3,
    OodleLZ_Decode_Unthreaded = OodleLZ_Decode_ThreadPhaseAll
} OodleLZ_Decode_ThreadPhase;
/* ThreadPhase for threaded Oodle decode

    Check $OodleLZ_Compressor_CanDecodeThreadPhased
    (currently only used by Kraken)

    See $OodleLZ_About_ThreadPhasedDecode

*/

typedef enum OodleLZ_FuzzSafe
{
    OodleLZ_FuzzSafe_No = 0,
    OodleLZ_FuzzSafe_Yes = 1
} OodleLZ_FuzzSafe;
/* OodleLZ_FuzzSafe (deprecated)

    About fuzz safety:

    Fuzz Safe decodes will not crash on corrupt data.  They may or may not return failure, and produce garbage output.

    Fuzz safe decodes will not read out of bounds.  They won't put data on the stack or previously in memory
    into the output buffer.

    As of Oodle 2.9.0 all compressors supported are fuzzsafe, so OodleLZ_FuzzSafe_Yes should always be used and this
    enum is deprecated.

*/

#define OODLELZ_BLOCK_LEN   (1<<18) /* The number of raw bytes per "seek chunk"
    Seek chunks can be decompressed independently if $(OodleLZ_CompressOptions:seekChunkReset) is set.
*/

#define OODLELZ_BLOCK_MAXIMUM_EXPANSION (2)
#define OODLELZ_BLOCK_MAX_COMPLEN       (OODLELZ_BLOCK_LEN+OODLELZ_BLOCK_MAXIMUM_EXPANSION) /* Maximum expansion per $OODLELZ_BLOCK_LEN is 1 byte.
    Note that the compressed buffer must be allocated bigger than this (use $OodleLZ_GetCompressedBufferSizeNeeded)
*/

#define OODLELZ_QUANTUM_LEN     (1<<14) /* Minimum decompression quantum (for old legacy codecs only)

    Deprecated.

    The new sea monster family of compressors use a whole block quantum (OODLELZ_BLOCK_LEN).
    Check $OodleLZ_Compressor_UsesWholeBlockQuantum
*/

// 5 byte expansion per-quantum with CRC's
#define OODLELZ_QUANTUM_MAXIMUM_EXPANSION   (5)

#define OODLELZ_QUANTUM_MAX_COMPLEN     (OODLELZ_QUANTUM_LEN+OODLELZ_QUANTUM_MAXIMUM_EXPANSION)

#define OODLELZ_SEEKCHUNKLEN_MIN        OODLELZ_BLOCK_LEN
#define OODLELZ_SEEKCHUNKLEN_MAX        (1<<29) // half GB

typedef OOSTRUCT OodleLZ_DecodeSome_Out
{
    OO_S32 decodedCount;    // number of uncompressed bytes decoded
    OO_S32 compBufUsed; // number of compressed bytes consumed


    OO_S32 curQuantumRawLen;  // tells you the current quantum size. you must have at least this much room available in the output buffer to be able to decode anything.
    OO_S32 curQuantumCompLen; // if you didn't pass in enough data, nothing will decode (decodedCount will be 0), and this will tell you how much is needed
} OodleLZ_DecodeSome_Out;
/* Output value of $OodleLZDecoder_DecodeSome
*/

//---------------------------------------------

//=======================================================

typedef OOSTRUCT OodleLZ_SeekTable
{
    OodleLZ_Compressor  compressor;             // which compressor was used
    OO_BOOL             seekChunksIndependent;  // are the seek chunks independent, or must they be decompressed in sequence

    OO_S64              totalRawLen;    // total uncompressed data lenth
    OO_S64              totalCompLen;   // sum of seekChunkCompLens

    OO_S32              numSeekChunks;  // derived from rawLen & seekChunkLen
    OO_S32              seekChunkLen;   // multiple of OODLELZ_BLOCK_LEN

    OO_U32 *            seekChunkCompLens;  // array of compressed lengths of seek chunks
    OO_U32 *            rawCRCs;            // crc of the raw bytes of the chunk (optional; NULL unless $OodleLZSeekTable_Flags_MakeRawCRCs was specified)
} OodleLZ_SeekTable;

typedef enum OodleLZSeekTable_Flags
{
    OodleLZSeekTable_Flags_None  = 0,       // default
    OodleLZSeekTable_Flags_MakeRawCRCs = 1,  // make the _rawCRCs_ member of $OodleLZ_SeekTable
    OodleLZSeekTable_Flags_Force32 = 0x40000000
} OodleLZSeekTable_Flags;

//=====================================================


typedef OOSTRUCT OodleConfigValues
{
    OO_S32 m_OodleLZ_LW_LRM_step;           // LZHLW LRM : bytes between LRM entries
    OO_S32 m_OodleLZ_LW_LRM_hashLength; // LZHLW LRM : bytes hashed for each LRM entries
    OO_S32 m_OodleLZ_LW_LRM_jumpbits;       // LZHLW LRM : bits of hash used for jump table

    OO_S32 m_OodleLZ_Decoder_Max_Stack_Size;    // if OodleLZ_Decompress needs to allocator a Decoder object, and it's smaller than this size, it's put on the stack instead of the heap
    OO_S32 m_OodleLZ_Small_Buffer_LZ_Fallback_Size_Unused; // deprecated
    OO_S32 m_OodleLZ_BackwardsCompatible_MajorVersion; // if you need to encode streams that can be read with an older version of Oodle, set this to the Oodle2 MAJOR version number that you need compatibility with.  eg to be compatible with oodle 2.7.3 you would put 7 here

    OO_U32 m_oodle_header_version;  // = OODLE_HEADER_VERSION

} OodleConfigValues;
/* OodleConfigValues

    Struct of user-settable low level config values.  See $Oodle_SetConfigValues.

    May have different defaults per platform.
*/

OOFUNC1 void OOFUNC2 Oodle_GetConfigValues(OodleConfigValues * ptr);
/* Get $OodleConfigValues

    $:ptr   filled with OodleConfigValues

    Gets the current $OodleConfigValues.

    May be different per platform.
*/

OOFUNC1 void OOFUNC2 Oodle_SetConfigValues(const OodleConfigValues * ptr);
/* Set $OodleConfigValues

    $:ptr   your desired OodleConfigValues

    Sets the global $OodleConfigValues from your struct.

    You should call $Oodle_GetConfigValues to fill the struct, then change the values you
    want to change, then call $Oodle_SetConfigValues.

    This should generally be done before doing anything with Oodle (eg. even before OodleX_Init).
    Changing OodleConfigValues after Oodle has started has undefined effects.
*/

typedef enum Oodle_UsageWarnings
{
    Oodle_UsageWarnings_Enabled = 0,
    Oodle_UsageWarnings_Disabled = 1,
    Oodle_UsageWarnings_Force32 = 0x40000000
} Oodle_UsageWarnings;
/* Whether Oodle usage warnings are enable or disabled. */

OOFUNC1 void OOFUNC2 Oodle_SetUsageWarnings(Oodle_UsageWarnings state);
/* Enables or disables Oodle usage warnings.

    $:state    whether usage warnings should be enabled or disabled.

   Usage warnings are enabled by default and try to be low-noise, but in case you want to
   disable them, this is how.

   This should generally be done once at startup.  Setting this state while there are Oodle
   calls running on other threads has undefined results.
*/

// function pointers to mallocs needed :

OODEFFUNC typedef void * (OODLE_CALLBACK t_fp_OodleCore_Plugin_MallocAligned)( OO_SINTa bytes, OO_S32 alignment);
/* Function pointer type for OodleMallocAligned

    $:bytes     number of bytes to allocate
    $:alignment required alignment of returned pointer
    $:return    pointer to memory allocated (must not be NULL)

    _alignment_ will always be a power of two

    _alignment_ will always be >= $OODLE_MALLOC_MINIMUM_ALIGNMENT

*/

OODEFFUNC typedef void (OODLE_CALLBACK t_fp_OodleCore_Plugin_Free)( void * ptr );
/* Function pointer type for OodleFree

    $:return    pointer to memory to free

*/

OOFUNC1 void OOFUNC2 OodleCore_Plugins_SetAllocators(
    t_fp_OodleCore_Plugin_MallocAligned * fp_OodleMallocAligned,
    t_fp_OodleCore_Plugin_Free * fp_OodleFree);
/* Set the function pointers for allocation needed by Oodle2 Core

    If these are not set, the default implementation on most platforms uses the C stdlib.
    On Microsoft platforms the default implementation uses HeapAlloc.

    These must not be changed once they are set!  Set them once then don't change them.

    NOTE: if you are using Oodle Ext, do NOT call this.  OodleX_Init will install an allocator for Oodle Core.  Do not mix your own allocator with the OodleX allocator.  See $OodleXAPI_Malloc.

    If you want to ensure that Oodle is not doing any allocations, you can call OodleCore_Plugins_SetAllocators(NULL,NULL);
    If you do that, then any time Oodle needs to allocate memory internally, it will stop the process.
    It is STRONGLY not recommended that you ship that way.  You can verify that Oodle is not allocating, but then leave some
    fallback allocator installed when you actually ship just in case.

    Also note that on many consoles the standard allocation practices may not
    leave much heap memory for the C stdlib malloc.  In this case Oodle may fail to allocate.

*/

OODEFFUNC typedef OO_U64 (OODLE_CALLBACK t_fp_OodleCore_Plugin_RunJob)( t_fp_Oodle_Job * fp_job, void * job_data , OO_U64 * dependencies, int num_dependencies, void * user_ptr );
/* Function pointer type for OodleCore_Plugins_SetJobSystem

    $:dependencies      array of handles of other pending jobs. All guaranteed to be nonzero.
    $:num_dependencies  number of dependencies. Guaranteed to be no more than OODLE_JOB_MAX_DEPENDENCIES.
    $:user_ptr          is passed through from the OodleLZ_CompressOptions.
    $:return            handle to the async job, or 0 if it was run synchronously

    RunJob will call fp_job(job_data)

    it may be done on a thread, or it may run the function synchronously and return 0, indicating the job is already done.
    The returned OO_U64 is a handle passed to WaitJob, unless it is 0, in which case WaitJob won't get called.

    fp_job should not run until all the dependencies are done.  This function should not delete the dependencies.

    See $Oodle_About_Job_Threading_Plugins
*/

OODEFFUNC typedef void (OODLE_CALLBACK t_fp_OodleCore_Plugin_WaitJob)( OO_U64 job_handle, void * user_ptr );
/* Function pointer type for OodleCore_Plugins_SetJobSystem

    $:job_handle    a job handle returned from RunJob. Never 0.
    $:user_ptr      is passed through from the OodleLZ_CompressOptions.

    Waits until the job specified by job_handle is done and cleans up any associated resources. Oodle
    will call WaitJob exactly once for every RunJob call that didn't return 0.

    If job_handle was already completed, this should clean it up without waiting.

    A handle value should not be reused by another RunJob until WaitJob has been done with that value.

    WaitJob will not be called from running jobs.  It will be only be called from the original thread that
    invoked Oodle.  If you are running Oodle from a worker thread, ensure that that thread is allowed to wait
    on other job threads.

    See $Oodle_About_Job_Threading_Plugins
*/

OOFUNC1 void OOFUNC2 OodleCore_Plugins_SetJobSystem(
    t_fp_OodleCore_Plugin_RunJob * fp_RunJob,
    t_fp_OodleCore_Plugin_WaitJob * fp_WaitJob);
/* DEPRECATED use OodleCore_Plugins_SetJobSystemAndCount instead

    See $OodleCore_Plugins_SetJobSystemAndCount
*/


OOFUNC1 void OOFUNC2 OodleCore_Plugins_SetJobSystemAndCount(
    t_fp_OodleCore_Plugin_RunJob * fp_RunJob,
    t_fp_OodleCore_Plugin_WaitJob * fp_WaitJob,
    int target_parallelism);
/* Set the function pointers for async job system needed by Oodle2 Core

    $:fp_RunJob     pointer to RunJob function
    $:fp_WaitJob    pointer to WaitJob function
    $:target_parallelism    goal of number of jobs to run simultaneously

    If these are not set, the default implementation runs jobs synchronously on the calling thread.

    These must not be changed once they are set!  Set them once then don't change them.

    _target_parallelism_ allows you to tell Oodle how many Jobs it should try to keep in flight at once.
    Depending on the operation it may not be able to split work into this many jobs (so fewer will be used),
    but it will not exceed this count.

    For Oodle Data LZ work, typically _target_parallelism_ is usually best at the number of hardware cores
    not including hyper threads).

    For Oodle Texture BCN encoding work, _target_parallelism_ is usually best as the full number of hyper cores.

    In some cases you may wish to reduce _target_parallelism_ by 1 or 2 cores to leave some of the CPU free for
    other work.

    For example on a CPU with 16 cores and 32 hardware threads, for LZ work you might set _target_parallelism_ to 15
    when calling OodleCorePlugins.  For BC7 encoding you might set _target_parallelism_ to 30 when calling OodleTexPlugins.

    NOTE : if you are using Oodle Ext, do NOT call this.  OodleX_Init will install a job system for Oodle Core.
    Note OodleX only installs automatically to Oodle Core, not Net or Tex.  See example_jobify.cpp for manual
    plugin.

    Replaces deprecated $OodleCore_Plugins_SetJobSystem

    See $Oodle_About_Job_Threading_Plugins
*/

// the main func pointer for log :
OODEFFUNC typedef void (OODLE_CALLBACK t_fp_OodleCore_Plugin_Printf)(int verboseLevel,const char * file,int line,const char * fmt,...);
/* Function pointer to Oodle Core printf

    $:verboseLevel  verbosity of the message; 0-2 ; lower = more important
    $:file          C file that sent the message
    $:line          C line that sent the message
    $:fmt           vararg printf format string

    The logging function installed here must parse varargs like printf.

    _verboseLevel_ may be used to omit verbose messages.
*/

OOFUNC1 t_fp_OodleCore_Plugin_Printf * OOFUNC2 OodleCore_Plugins_SetPrintf(t_fp_OodleCore_Plugin_Printf * fp_rrRawPrintf);
/* Install the callback used by Oodle Core for logging

    $:fp_rrRawPrintf    function pointer to your log function; may be NULL to disable all logging
    $:return            returns the previous function pointer

    Use this function to install your own printf for Oodle Core.

    The default implementation in debug builds, if you install nothing, uses the C stdio printf for logging.
    On Microsoft platforms, it uses OutputDebugString and not stdio.

    To disable all logging, call OodleCore_Plugins_SetPrintf(NULL)

    WARNING : this function is NOT thread safe!  It should be done only once and done in a place where the caller can guarantee thread safety.

    In the debug build of Oodle, you can install OodleCore_Plugin_Printf_Verbose to get more verbose logging

*/

OODEFFUNC typedef OO_BOOL (OODLE_CALLBACK t_fp_OodleCore_Plugin_DisplayAssertion)(const char * file,const int line,const char * function,const char * message);
/* Function pointer to Oodle Core assert callback

    $:file          C file that triggered the assert
    $:line          C line that triggered the assert
    $:function      C function that triggered the assert (may be NULL)
    $:message       assert message
    $:return        true to break execution at the assertion site, false to continue

    This callback is called by Oodle Core when it detects an assertion condition.

    This will only happen in debug builds.


*/

OOFUNC1 t_fp_OodleCore_Plugin_DisplayAssertion * OOFUNC2 OodleCore_Plugins_SetAssertion(t_fp_OodleCore_Plugin_DisplayAssertion * fp_rrDisplayAssertion);
/* Install the callback used by Oodle Core for asserts

    $:fp_rrDisplayAssertion function pointer to your assert display function
    $:return            returns the previous function pointer

    Use this function to install your own display for Oodle Core assertions.
    This will only happen in debug builds.

    The default implementation in debug builds, if you install nothing, uses the C stderr printf for logging,
    except on Microsoft platforms where it uses OutputDebugString.

    WARNING : this function is NOT thread safe!  It should be done only once and done in a place where the caller can guarantee thread safety.

*/

//=============================================================


OOFUNC1 void * OOFUNC2 OodleCore_Plugin_MallocAligned_Default(OO_SINTa size,OO_S32 alignment);
OOFUNC1 void OOFUNC2 OodleCore_Plugin_Free_Default(void * ptr);
OOFUNC1 void OOFUNC2 OodleCore_Plugin_Printf_Default(int verboseLevel,const char * file,int line,const char * fmt,...);
OOFUNC1 void OOFUNC2 OodleCore_Plugin_Printf_Verbose(int verboseLevel,const char * file,int line,const char * fmt,...);
OOFUNC1 OO_BOOL OOFUNC2 OodleCore_Plugin_DisplayAssertion_Default(const char * file,const int line,const char * function,const char * message);
OOFUNC1 OO_U64 OOFUNC2 OodleCore_Plugin_RunJob_Default( t_fp_Oodle_Job * fp_job, void * job_data, OO_U64 * dependencies, int num_dependencies, void * user_ptr );
OOFUNC1 void OOFUNC2 OodleCore_Plugin_WaitJob_Default( OO_U64 job_handle, void * user_ptr );

//=============================================================

//----------------------------------------------
// OodleLZ

#define OODLELZ_FAILED      (0) /* Return value of OodleLZ_Decompress on failure
*/

//=======================================================

OOFUNC1 OO_SINTa OOFUNC2 OodleLZ_Compress(OodleLZ_Compressor compressor,
    const void * rawBuf,OO_SINTa rawLen,void * compBuf,
    OodleLZ_CompressionLevel level,
    const OodleLZ_CompressOptions * pOptions OODEFAULT(NULL),
    const void * dictionaryBase OODEFAULT(NULL),
    const void * lrm OODEFAULT(NULL),
    void * scratchMem OODEFAULT(NULL),
    OO_SINTa scratchSize OODEFAULT(0) );
/* Compress some data from memory to memory, synchronously, with OodleLZ

    $:compressor    which OodleLZ variant to use in compression
    $:rawBuf        raw data to compress
    $:rawLen        number of bytes in rawBuf to compress
    $:compBuf       pointer to write compressed data to ; should be at least $OodleLZ_GetCompressedBufferSizeNeeded
    $:level         OodleLZ_CompressionLevel controls how much CPU effort is put into maximizing compression
    $:pOptions          (optional) options; if NULL, $OodleLZ_CompressOptions_GetDefault is used
    $:dictionaryBase    (optional) if not NULL, provides preceding data to prime the dictionary; must be contiguous with rawBuf, the data between the pointers _dictionaryBase_ and _rawBuf_ is used as the preconditioning data.  The exact same precondition must be passed to encoder and decoder.
    $:lrm               (optional) long range matcher
    $:scratchMem        (optional) pointer to scratch memory
    $:scratchSize       (optional) size of scratch memory (see $OodleLZ_GetCompressScratchMemBound)
    $:return    size of compressed data written, or $OODLELZ_FAILED for failure

    Performs synchronous memory to memory LZ compression.

    In tools, you should generally use $OodleXLZ_Compress_AsyncAndWait instead to get parallelism.  (in the Oodle2 Ext lib)

    You can compress a large buffer in several calls by setting _dictionaryBase_ to the start
    of the buffer, and then making _rawBuf_ and _rawLen_ select portions of that buffer.  As long
    as _rawLen_ is a multiple of $OODLELZ_BLOCK_LEN , the compressed chunks can simply be
    concatenated together.

    If _scratchMem_ is provided, it will be used for the compressor's scratch memory needs before OodleMalloc is
    called.  If the scratch is big enough, no malloc will be done.  If the scratch is not big enough, the compress
    will not fail, instead OodleMalloc will be used.  OodleMalloc should not return null.  There is currently no way
    to make compress fail cleanly due to using too much memory, it must either succeed or abort the process.

    If _scratchSize_ is at least $OodleLZ_GetCompressScratchMemBound , additional allocations will not be needed.

    See $OodleLZ_About for tips on setting the compression options.

    If _dictionaryBase_ is provided, the backup distance from _rawBuf_ must be a multiple of $OODLELZ_BLOCK_LEN

    If $(OodleLZ_CompressOptions:seekChunkReset) is enabled, and _dictionaryBase_ is not NULL or _rawBuf_ , then the
    seek chunk boundaries are relative to _dictionaryBase_, not to _rawBuf_.

*/

// Decompress returns raw (decompressed) len received
// Decompress returns 0 (OODLELZ_FAILED) if it detects corruption
OOFUNC1 OO_SINTa OOFUNC2 OodleLZ_Decompress(const void * compBuf,OO_SINTa compBufSize,void * rawBuf,OO_SINTa rawLen,
                                            OodleLZ_FuzzSafe fuzzSafe OODEFAULT(OodleLZ_FuzzSafe_Yes),
                                            OodleLZ_CheckCRC checkCRC OODEFAULT(OodleLZ_CheckCRC_No),
                                            OodleLZ_Verbosity verbosity OODEFAULT(OodleLZ_Verbosity_None),
                                            void * decBufBase OODEFAULT(NULL),
                                            OO_SINTa decBufSize OODEFAULT(0),
                                            OodleDecompressCallback * fpCallback OODEFAULT(NULL),
                                            void * callbackUserData OODEFAULT(NULL),
                                            void * decoderMemory OODEFAULT(NULL),
                                            OO_SINTa decoderMemorySize OODEFAULT(0),
                                            OodleLZ_Decode_ThreadPhase threadPhase OODEFAULT(OodleLZ_Decode_Unthreaded)
                                            );
/* Decompress a some data from memory to memory, synchronously.

    $:compBuf       pointer to compressed data
    $:compBufSize   number of compressed bytes available (must be greater or equal to the number consumed)
    $:rawBuf        pointer to output uncompressed data into
    $:rawLen        number of uncompressed bytes to output
    $:fuzzSafe      (optional) should the decode fail if it contains non-fuzz safe codecs?
    $:checkCRC      (optional) if data could be corrupted and you want to know about it, pass OodleLZ_CheckCRC_Yes
    $:verbosity     (optional) if not OodleLZ_Verbosity_None, logs some info
    $:decBufBase    (optional) if not NULL, provides preceding data to prime the dictionary; must be contiguous with rawBuf, the data between the pointers _dictionaryBase_ and _rawBuf_ is used as the preconditioning data.   The exact same precondition must be passed to encoder and decoder.  The decBufBase must be a reset point.
    $:decBufSize    (optional) size of decode buffer starting at decBufBase, if 0, _rawLen_ is assumed
    $:fpCallback    (optional) OodleDecompressCallback to call incrementally as decode proceeds
    $:callbackUserData (optional) passed as userData to fpCallback
    $:decoderMemory (optional) pre-allocated memory for the Decoder, of size _decoderMemorySize_
    $:decoderMemorySize (optional) size of the buffer at _decoderMemory_; must be at least $OodleLZDecoder_MemorySizeNeeded bytes to be used
    $:threadPhase   (optional) for threaded decode; see $OodleLZ_About_ThreadPhasedDecode (default OodleLZ_Decode_Unthreaded)
    $:return        the number of decompressed bytes output, $OODLELZ_FAILED (0) if none can be decompressed

    Decodes data encoded with any $OodleLZ_Compressor.

    Note : _rawLen_ must be the actual number of bytes to output, the same as the number that were encoded with the corresponding
    OodleLZ_Compress size.  You must store this somewhere in your own header and pass it in to this call.  _compBufSize_ does NOT
    need to be the exact number of compressed bytes, is the number of bytes available in the buffer, it must be greater or equal to
    the actual compressed length.

    Note that the new compressors (Kraken,Mermaid,Selkie,BitKnit) are all fuzz safe and you can use OodleLZ_FuzzSafe_Yes
    with them and no padding of the decode target buffer.

    If checkCRC is OodleLZ_CheckCRC_Yes, then corrupt data will be detected and the decode aborted.
    If checkCRC is OodleLZ_CheckCRC_No, then corruption might result in invalid data, but no detection of any error (garbage in, garbage out).

    If corruption is possible, _fuzzSafe_ is No and _checkCRC_ is OodleLZ_CheckCRC_No, $OodleLZ_GetDecodeBufferSize must be used to allocate
    _rawBuf_ large enough to prevent overrun.

    $OodleLZ_GetDecodeBufferSize should always be used to ensure _rawBuf_ is large enough, even when corruption is not
    possible (when fuzzSafe is No).

    _compBuf_ and _rawBuf_ are allowed to overlap for "in place" decoding, but then _rawBuf_ must be allocated to
    the size given by $OodleLZ_GetInPlaceDecodeBufferSize , and the compressed data must be at the end of that buffer.

    An easy way to take the next step to parallel decoding is with $OodleXLZ_Decompress_MakeSeekTable_Wide_Async (in the Oodle2 Ext lib)

    NOTE : the return value is the *total* number of decompressed bytes output so far.  If rawBuf is > decBufBase, that means
    the initial inset of (rawBuf - decBufBase) is included!  (eg. you won't just get _rawLen_)
    
    If _decBufBase_ is provided, the backup distance from _rawBuf_ must be a multiple of $OODLELZ_BLOCK_LEN

    About fuzz safety:
    
    OodleLZ_Decompress is guaranteed not to crash even if the data is corrupted when _fuzzSafe_ is set to OodleLZ_FuzzSafe_Yes.
    When _fuzzSafe_ is Yes, the target buffer (_rawBuf_ and _rawLen_) will never be overrun.  Note that corrupted day might not
    be detected (the return value might indicate success).

    Fuzz Safe decodes will not crash on corrupt data.  They may or may not return failure, and produce garbage output.

    Fuzz safe decodes will not read out of bounds.  They won't put data on the stack or previously in memory
    into the output buffer.

    Fuzz safe decodes will not output more than the uncompressed size. (eg. the output buffer does not need to
    be padded like OodleLZ_GetDecodeBufferSize)

    If you ask for a fuzz safe decode and the compressor doesn't satisfy OodleLZ_Compressor_CanDecodeFuzzSafe
    then it will return failure.

    The _fuzzSafe_ argument should always be OodleLZ_FuzzSafe_Yes as of Oodle 2.9.0 ; older compressors did not
    support fuzz safety but they now all do.

    Use of OodleLZ_FuzzSafe_No is deprecated.
    
*/


//-------------------------------------------
// Incremental Decoder functions :

struct _OodleLZDecoder;
typedef struct _OodleLZDecoder OodleLZDecoder;
/* Opaque type for OodleLZDecoder

    See $OodleLZDecoder_Create
*/


OOFUNC1 OodleLZDecoder * OOFUNC2 OodleLZDecoder_Create(OodleLZ_Compressor compressor,OO_S64 rawLen,void * memory, OO_SINTa memorySize);
/*  Create a OodleLZDecoder

    $:compressor the type of data you will decode; use $OodleLZ_Compressor_Invalid if unknown
    $:rawLen    total raw bytes of the decode
    $:memory    (optional) provide memory for the OodleLZDecoder object (not the window)
    $:memorySize (optional) if memory is provided, this is its size in bytes
    $:return    the OodleLZDecoder

    If memory is provided, it must be of size $OodleLZDecoder_MemorySizeNeeded.  If it is NULL it will be
    allocated with the malloc specified by $OodleAPI_OodleCore_Plugins.

    Free with $OodleLZDecoder_Destroy.  You should Destroy even if you passed in the memory.

    Providing _compressor_ lets the OodleLZDecoder be the minimum size needed for that type of data.
    If you pass $OodleLZ_Compressor_Invalid, then any type of data may be decoded, and the Decoder is allocated
    large enought to handle any of them.

    If you are going to pass rawLen to OodleLZDecoder_Reset , then you can pass 0 to rawLen here.

    See $OodleLZDecoder_DecodeSome for more.
*/

OOFUNC1 OO_S32 OOFUNC2 OodleLZDecoder_MemorySizeNeeded(OodleLZ_Compressor compressor OODEFAULT(OodleLZ_Compressor_Invalid), OO_SINTa rawLen OODEFAULT(-1));
/* If you want to provide the memory needed by $OodleLZDecoder_Create , this tells you how big it must be.

    $:compressor the type of data you will decode; use $OodleLZ_Compressor_Invalid if unknown
    $:rawLen    should almost always be -1, which supports any size of raw data decompression
    $:return    bytes to allocate or reserve, 0 for failure

    NOTE : using $OodleLZ_Compressor_Invalid lets you decode any time of compressed data.
    It requests as much memory as the largest compressor. This may be a *lot* more than your data needs;
    try to use the correct compressor type.

    If _rawLen_ is -1 (default) then the Decoder object created can be used on any length of raw data
    decompression.  If _rawLen_ is specified here, then you can only use it to decode data shorter than
    the length you specified here.  This use case is very rare, contact support for details.
*/

OOFUNC1 OO_S32 OOFUNC2 OodleLZ_ThreadPhased_BlockDecoderMemorySizeNeeded(void);
/* Returns the size of the decoder needed for ThreadPhased decode

    For use with $OodleLZ_Decode_ThreadPhase
    See $OodleLZ_About_ThreadPhasedDecode
*/

OOFUNC1 void OOFUNC2 OodleLZDecoder_Destroy(OodleLZDecoder * decoder);
/* Pairs with $OodleLZDecoder_Create

    You should always call Destroy even if you provided the memory for $OodleLZDecoder_Create
*/

// Reset decoder - can reset to the start of any OODLELZ_BLOCK_LEN chunk
OOFUNC1 OO_BOOL OOFUNC2 OodleLZDecoder_Reset(OodleLZDecoder * decoder, OO_SINTa decPos, OO_SINTa decLen OODEFAULT(0));
/* Reset an OodleLZDecoder to restart at given pos

    $:decoder   the OodleLZDecoder, made by $OodleLZDecoder_Create
    $:decPos    position to reset to; must be a multiple of OODLELZ_BLOCK_LEN
    $:decLen    (optional) if not zero, change the length of the data we expect to decode
    $:return    true for success

    If you are seeking in a packed stream, you must seek to a seek chunk reset point, as was made at compress time.

    That is, $(OodleLZ_CompressOptions:seekChunkReset) must have been true, and
    _decPos_ must be a multiple of $(OodleLZ_CompressOptions:seekChunkLen) that was used at compress time.

    You can use $OodleLZ_GetChunkCompressor to verify that you are at a valid
    independent chunk start point.

*/

struct LZBlockHeader
{
    int version;
    int decodeType;
    int offsetShift;
    int chunkIsMemcpy;
    int chunkIsReset;
    int chunkHasQuantumCRCs;
};


// returns false if corruption detected
OOFUNC1 OO_BOOL OOFUNC2 OodleLZDecoder_DecodeSome(
    OodleLZDecoder* decoder,
    OodleLZ_DecodeSome_Out* out,

    // the decode sliding window : we output here & read from this for matches
    void* decBuf,
    OO_SINTa decBufPos,
    OO_SINTa decBufferSize,  // decBufferSize should be the result of OodleLZDecoder_MakeDecodeBufferSize()
    OO_SINTa decBufAvail, // usually Size - Pos, but maybe less if you have pending IO flushes

    // compressed data :
    const void* compPtr,
    OO_SINTa compAvail,

    OodleLZ_FuzzSafe fuzzSafe OODEFAULT(OodleLZ_FuzzSafe_No),
    OodleLZ_CheckCRC checkCRC OODEFAULT(OodleLZ_CheckCRC_No),
    OodleLZ_Verbosity verbosity OODEFAULT(OodleLZ_Verbosity_None),
    OodleLZ_Decode_ThreadPhase threadPhase OODEFAULT(OodleLZ_Decode_Unthreaded)

);
/* Incremental decode some LZ compressed data

    $:decoder   the OodleLZDecoder, made by $OodleLZDecoder_Create
    $:out       filled with results
    $:decBuf    the decode buffer (window)
    $:decBufPos the current position in the buffer
    $:decBufferSize size of decBuf ; this must be either equal to the total decompressed size (_rawLen_ passed to $OodleLZDecoder_Create) or the result of $OodleLZDecoder_MakeValidCircularWindowSize
    $:decBufAvail   the number of bytes available after decBufPos in decBuf ; usually (decBufferSize - decBufPos), but can be less
    $:compPtr   pointer to compressed data to read
    $:compAvail number of compressed bytes available at compPtr
    $:fuzzSafe      (optional) should the decode be fuzz safe
    $:checkCRC      (optional) if data could be corrupted and you want to know about it, pass OodleLZ_CheckCRC_Yes
    $:verbosity     (optional) if not OodleLZ_Verbosity_None, logs some info
    $:threadPhase   (optional) for threaded decode; see $OodleLZ_About_ThreadPhasedDecode (default OodleLZ_Decode_Unthreaded)
    $:return    true if success, false if invalid arguments or data is encountered

    Decodes data encoded with an OodleLZ compressor.

    Decodes an integer number of quanta; quanta are $OODLELZ_QUANTUM_LEN uncompressed bytes.

    _decBuf_ can either be a circular window or the whole _rawLen_ array.
    In either case, _decBufPos_ should be in the range [0,_decBufferSize_).
    If _decBuf_ is a circular window, then _decBufferSize_ should come from $OodleLZDecoder_MakeValidCircularWindowSize.

    (circular windows are deprecated as of 2.9.0)

    NOTE : all the new LZ codecs (Kraken, etc.) do not do circular windows.  They can do sliding windows, see lz_test_11 in $example_lz.
    They should always have decBufferSize = total raw size, even if the decode buffer is smaller than that.

    NOTE : insufficient data provided (with _compAvail_ > 0 but not enough to decode a quantum) is a *success* case
    (return value of true), even though nothing is decoded.  A return of false always indicates a non-recoverable error.

    If _decBufAvail_ or _compAvail_ is insufficient for any decompression, the "curQuantum" fields of $OodleLZ_DecodeSome_Out
    will tell you how much you must provide to proceed.  That is, if enough compressed bytes are provided to get a quantum header, but not enough to decode a quantum, this
    function returns true and fills out the $OodleLZ_DecodeSome_Out structure with the size of the quantum.

    See $OodleLZ_Decompress about fuzz safety.

    NOTE : DecodeSome expect to decode either one full quantum (of len $OODLELZ_QUANTUM_LEN) or up to the length of the total buffer specified in the
call to $OodleLZDecoder_Create or $OodleLZDecoder_Reset.  That total buffer length
must match what was use during compression (or be a seek-chunk portion thereof).
That is, you cannot decompress partial streams in intervals smaller than
$OODLELZ_QUANTUM_LEN except for the final partial quantum at the end of the stream.

*/

// pass in how much you want to alloc and it will tell you a valid size as close that as possible
//  the main use is just to call OodleLZDecoder_MakeDecodeBufferSize(0) to get the min size; the min size is a good size
OOFUNC1 OO_S32 OOFUNC2 OodleLZDecoder_MakeValidCircularWindowSize(OodleLZ_Compressor compressor,OO_S32 minWindowSize OODEFAULT(0));
/* Get a valid "Window" size for an LZ

    $:compressor    which compressor you will be decoding
    $:minWindowSize (optional) minimum size of the window

    NOTE: circular windows are deprecated as of 2.9.0

    Most common usage is OodleLZDecoder_MakeValidCircularWindowSize(0) to get the minimum window size.

    Only compressors which pass $OodleLZ_Compressor_CanDecodeInCircularWindow can be decoded in a circular window.

    WARNING : this is NOT the size to malloc the window! you need to call $OodleLZ_GetDecodeBufferSize() and
    pass in the window size to get the malloc size.
*/

//=======================================================

//=======================================================
// remember if you want to IO the SeekEntries you need to make them endian-independent
//  see WriteOOZHeader for example

#define OODLELZ_SEEKPOINTCOUNT_DEFAULT  16

OOFUNC1 OO_S32 OOFUNC2 OodleLZ_MakeSeekChunkLen(OO_S64 rawLen, OO_S32 desiredSeekPointCount);
/* Compute a valid seekChunkLen

    $:rawLen    total length of uncompressed data
    $:desiredSeekPointCount desired number of seek chunks
    $:return    a valid seekChunkLen for use in $OodleLZ_CreateSeekTable

    Returns a seekChunkLen which is close to (rawLen/desiredSeekPointCount) but is a power of two multiple of $OODLELZ_BLOCK_LEN

    _desiredSeekPointCount_ = 16 is good for parallel decompression.
    (OODLELZ_SEEKPOINTCOUNT_DEFAULT)
*/

OOFUNC1 OO_S32 OOFUNC2 OodleLZ_GetNumSeekChunks(OO_S64 rawLen, OO_S32 seekChunkLen);
/* Compute the number of seek chunks

    $:rawLen        total length of uncompressed data
    $:seekChunkLen  the length of a seek chunk (eg from $OodleLZ_MakeSeekChunkLen)
    $:return        the number of seek chunks

    returns (rawLen+seekChunkLen-1)/seekChunkLen
*/

OOFUNC1 OO_SINTa OOFUNC2 OodleLZ_GetSeekTableMemorySizeNeeded(OO_S32 numSeekChunks,OodleLZSeekTable_Flags flags);
/* Tells you the size in bytes to allocate the seekTable before calling $OodleLZ_FillSeekTable

    $:numSeekChunks number of seek chunks (eg from $OodleLZ_GetNumSeekChunks)
    $:flags         options that will be passed to $OodleLZ_CreateSeekTable
    $:return        size in bytes of memory needed for seek table

    If you wish to provide the memory for the seek table yourself, you may call this to get the required size,
    allocate the memory, and then simply point a $OodleLZ_SeekTable at your memory.
    Then use $OodleLZ_FillSeekTable to fill it out.

    Do NOT use sizeof(OodleLZ_SeekTable) !
*/

OOFUNC1 OO_BOOL OOFUNC2 OodleLZ_FillSeekTable(OodleLZ_SeekTable * pTable,OodleLZSeekTable_Flags flags,OO_S32 seekChunkLen,const void * rawBuf, OO_SINTa rawLen,const void * compBuf,OO_SINTa compLen);
/* scan compressed LZ stream to fill the seek table

    $:pTable    pointer to table to be filled
    $:flags     options
    $:seekChunkLen  the length of a seek chunk (eg from $OodleLZ_MakeSeekChunkLen)
    $:rawBuf    (optional) uncompressed buffer; used to compute the _rawCRCs_ member of $OodleLZ_SeekTable
    $:rawLen    size of rawBuf
    $:compBuf   compressed buffer
    $:compLen   size of compBuf
    $:return    true for success

    _pTable_ must be able to hold at least $OodleLZ_GetSeekTableMemorySizeNeeded

    _seekChunkLen_ must be a multiple of $OODLELZ_BLOCK_LEN.
    _seekChunkLen_ must match what was in CompressOptions when the buffer was made, or any integer multiple thereof.
*/


OOFUNC1 OodleLZ_SeekTable * OOFUNC2 OodleLZ_CreateSeekTable(OodleLZSeekTable_Flags flags,OO_S32 seekChunkLen,const void * rawBuf, OO_SINTa rawLen,const void * compBuf,OO_SINTa compLen);
/* allocate a table, then scan compressed LZ stream to fill the seek table

    $:flags     options
    $:seekChunkLen  the length of a seek chunk (eg from $OodleLZ_MakeSeekChunkLen)
    $:rawBuf    (optional) uncompressed buffer; used to compute the _rawCRCs_ member of $OodleLZ_SeekTable
    $:rawLen    size of rawBuf
    $:compBuf   compressed buffer
    $:compLen   size of compBuf
    $:return    pointer to table if succeeded, null if failed

    Same as $OodleLZ_FillSeekTable , but allocates the memory for you.  Use $OodleLZ_FreeSeekTable to free.

    _seekChunkLen_ must be a multiple of $OODLELZ_BLOCK_LEN.
    _seekChunkLen_ must match what was in CompressOptions when the buffer was made, or any integer multiple thereof.

*/

OOFUNC1 void OOFUNC2 OodleLZ_FreeSeekTable(OodleLZ_SeekTable * pTable);
/* Frees a table allocated by $OodleLZ_CreateSeekTable
*/

OOFUNC1 OO_BOOL OOFUNC2 OodleLZ_CheckSeekTableCRCs(const void * rawBuf,OO_SINTa rawLen, const OodleLZ_SeekTable * seekTable);
/* Check the CRC's in seekTable vs rawBuf

    $:rawBuf    uncompressed buffer
    $:rawLen    size of rawBuf
    $:seekTable result of $OodleLZ_CreateSeekTable
    $:return    true if the CRC's check out

    Note that $OodleLZ_Decompress option of $OodleLZ_CheckCRC checks the CRC of *compressed* data,
    this call checks the CRC of the *raw* (uncompressed) data.

    OodleLZ data contains a CRC of the compressed data if it was made with $(OodleLZ_CompressOptions:sendQuantumCRCs).
    The SeekTable contains a CRC of the raw data if it was made with $OodleLZSeekTable_Flags_MakeRawCRCs.

    Checking the CRC of compressed data is faster, but does not verify that the decompress succeeded.
*/

OOFUNC1 OO_S32 OOFUNC2 OodleLZ_FindSeekEntry( OO_S64 rawPos, const OodleLZ_SeekTable * seekTable);
/* Find the seek entry that contains a raw position

    $:rawPos            uncompressed position to look for
    $:seekTable         result of $OodleLZ_CreateSeekTable
    $:return            a seek entry index

    returns the index of the chunk that contains _rawPos_
*/

OOFUNC1 OO_S64 OOFUNC2 OodleLZ_GetSeekEntryPackedPos( OO_S32 seekI , const OodleLZ_SeekTable * seekTable );
/* Get the compressed position of a seek entry

    $:seekI         seek entry index , in [0,numSeekEntries)
    $:seekTable     result of $OodleLZ_CreateSeekTable
    $:return        compressed buffer position of the start of this seek entry


*/

//=============================================================

OOFUNC1 const char * OOFUNC2 OodleLZ_CompressionLevel_GetName(OodleLZ_CompressionLevel compressSelect);
/* Provides a string naming a $OodleLZ_CompressionLevel compressSelect
*/

OOFUNC1 const char * OOFUNC2 OodleLZ_Compressor_GetName(OodleLZ_Compressor compressor);
/* Provides a string naming a $OodleLZ_Compressor compressor
*/

OOFUNC1 const char * OOFUNC2 OodleLZ_Jobify_GetName(OodleLZ_Jobify jobify);
/* Provides a string naming a $OodleLZ_Jobify enum
*/

OOFUNC1 const OodleLZ_CompressOptions * OOFUNC2 OodleLZ_CompressOptions_GetDefault(
                                                    OodleLZ_Compressor compressor OODEFAULT(OodleLZ_Compressor_Invalid),
                                                    OodleLZ_CompressionLevel lzLevel OODEFAULT(OodleLZ_CompressionLevel_Normal));
/* Provides a pointer to default compression options

    $:compressor    deprecated, ignored
    $:lzLevel       deprecated, ignored

    Use to fill your own $OodleLZ_CompressOptions then change individual fields.

*/

// after you fiddle with options, call this to ensure they are allowed
OOFUNC1 void OOFUNC2 OodleLZ_CompressOptions_Validate(OodleLZ_CompressOptions * pOptions);
/* Clamps the values in _pOptions_ to be in valid range

*/

// inline functions for compressor property queries
OODEFSTART

OO_BOOL OodleLZ_Compressor_UsesWholeBlockQuantum(OodleLZ_Compressor compressor);
/* OodleLZ_Compressor properties helper.

    Tells you if this compressor is "whole block quantum" ; must decode in steps of
    $OODLELZ_BLOCK_LEN , not $OODLELZ_QUANTUM_LEN like others.
*/
OO_BOOL OodleLZ_Compressor_UsesLargeWindow(OodleLZ_Compressor compressor);
/* OodleLZ_Compressor properties helper.

    Tells you if this compressor is "LargeWindow" or not, meaning it can benefit from
    a Long-Range-Matcher and windows larger than $OODLELZ_BLOCK_LEN
*/
OO_BOOL OodleLZ_Compressor_CanDecodeInCircularWindow(OodleLZ_Compressor compressor);
/* OodleLZ_Compressor properties helper.

    Tells you if this compressor can be decoded using a fixed size circular window.
    deprecated as of 2.9.0
*/
OO_BOOL OodleLZ_Compressor_CanDecodeThreadPhased(OodleLZ_Compressor compressor);
/* OodleLZ_Compressor properties helper.

    Tells you if this compressor can be used with the $OodleLZ_Decode_ThreadPhase.

    See $OodleLZ_About_ThreadPhasedDecode
*/
OO_BOOL OodleLZ_Compressor_CanDecodeInPlace(OodleLZ_Compressor compressor);
/* OodleLZ_Compressor properties helper.

    Tells you if this compressor can be used with "in-place" decoding.

    This is now always true (all compressors support in-place decoding).  The function is left
    for backward compatibility.

    All compressors in the future will support in-place, you don't need to check this property.

*/
OO_BOOL OodleLZ_Compressor_MustDecodeWithoutResets(OodleLZ_Compressor compressor);
/* OodleLZ_Compressor properties helper.

    Tells you if this compressor must decode contiguous ranges of buffer with the same Decoder.

    That is, most of the compressors can be Reset and restart on any block, not just seek blocks,
    as long as the correct window data is provided.  That is, if this returns false then the only
    state required across a non-reset block is the dictionary of previously decoded data.

    But if OodleLZ_Compressor_MustDecodeWithoutResets returns true, then you cannot do that,
    because the Decoder object must carry state across blocks (except reset blocks).

    This does not apply to seek points - you can always reset and restart decompression at a seek point.
*/
OO_BOOL OodleLZ_Compressor_CanDecodeFuzzSafe(OodleLZ_Compressor compressor);
/* OodleLZ_Compressor properties helper.

    Tells you if this compressor is "fuzz safe" which means it can accept corrupted data
    and won't crash or overrun any buffers.
*/

OO_BOOL OodleLZ_Compressor_RespectsDictionarySize(OodleLZ_Compressor compressor);
/* OodleLZ_Compressor properties helper.

    Tells you if this compressor obeys $(OodleLZ_CompressOptions:dictionarySize) which limits
    match references to a finite bound.  (eg. for sliding window decompression).

    All the new codecs do (Kraken,Mermaid,Selkie,Leviathan).  Some old codecs don't.
*/
//=====================================================================

#define OODLELZ_COMPRESSOR_MASK(c)  (((OO_U32)1)<<((OO_S32)(c)))
// OODLELZ_COMPRESSOR_BOOLBIT : extract a value of 1 or 0 so it maps to "bool"
#define OODLELZ_COMPRESSOR_BOOLBIT(s,c) (((s)>>(OO_S32)(c))&1)

OOINLINEFUNC OO_BOOL OodleLZ_Compressor_IsNewLZFamily(OodleLZ_Compressor compressor)
{
    const OO_U32 set =
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Kraken) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Leviathan) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Mermaid) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Selkie) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Hydra);
    return OODLELZ_COMPRESSOR_BOOLBIT(set,compressor);
}

OOINLINEFUNC OO_BOOL OodleLZ_Compressor_CanDecodeFuzzSafe(OodleLZ_Compressor compressor)
{
    #ifdef OODLE_ALLOW_DEPRECATED_COMPRESSORS
    const OO_U32 set =
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Kraken) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Leviathan) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Mermaid) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Selkie) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Hydra) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_BitKnit) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_LZB16);
    return OODLELZ_COMPRESSOR_BOOLBIT(set,compressor);
    #else
    // all new compressors are fuzz safe
    return compressor != OodleLZ_Compressor_Invalid;
    #endif
}

OOINLINEFUNC OO_BOOL OodleLZ_Compressor_RespectsDictionarySize(OodleLZ_Compressor compressor)
{
    #ifdef OODLE_ALLOW_DEPRECATED_COMPRESSORS
    const OO_U32 set =
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Kraken) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Leviathan) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Mermaid) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Selkie) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_Hydra) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_LZNA) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_BitKnit);
    return OODLELZ_COMPRESSOR_BOOLBIT(set,compressor);
    #else
    // all new compressors respect dictionarySize
    return compressor != OodleLZ_Compressor_Invalid;
    #endif
}

OOINLINEFUNC OO_BOOL OodleLZ_Compressor_UsesWholeBlockQuantum(OodleLZ_Compressor compressor)
{
    return OodleLZ_Compressor_IsNewLZFamily(compressor);
}

OOINLINEFUNC OO_BOOL OodleLZ_Compressor_CanDecodeThreadPhased(OodleLZ_Compressor compressor)
{
    return OodleLZ_Compressor_IsNewLZFamily(compressor);
}

OOINLINEFUNC OO_BOOL OodleLZ_Compressor_CanDecodeInPlace(OodleLZ_Compressor compressor)
{
    // all compressors can now decode in place :
    return compressor != OodleLZ_Compressor_Invalid;
}

OOINLINEFUNC OO_BOOL OodleLZ_Compressor_CanDecodeInCircularWindow(OodleLZ_Compressor compressor)
{
    #ifdef OODLE_ALLOW_DEPRECATED_COMPRESSORS
    const OO_U32 set =
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_LZH) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_LZB16);
    #else
    const OO_U32 set = 0;
    #endif

    return OODLELZ_COMPRESSOR_BOOLBIT(set,compressor);
}

OOINLINEFUNC OO_BOOL OodleLZ_Compressor_UsesLargeWindow(OodleLZ_Compressor compressor)
{
    // all but LZH and LZB16 now are large window
    return ! OodleLZ_Compressor_CanDecodeInCircularWindow(compressor);
}

OOINLINEFUNC OO_BOOL OodleLZ_Compressor_MustDecodeWithoutResets(OodleLZ_Compressor compressor)
{
    #ifdef OODLE_ALLOW_DEPRECATED_COMPRESSORS
    const OO_U32 set =
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_BitKnit) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_LZA) |
        OODLELZ_COMPRESSOR_MASK(OodleLZ_Compressor_LZNA);
    #else
    const OO_U32 set = 0;
    #endif

    return OODLELZ_COMPRESSOR_BOOLBIT(set,compressor);
}

OODEFEND

//=======================================================


#define OODLELZ_SCRATCH_MEM_NO_BOUND        (-1)    /* Scratch mem size when bound is unknown.
    Installed allocator may be used no matter how much scratch mem you provide.
*/

OOFUNC1 OO_SINTa OOFUNC2 OodleLZ_GetCompressScratchMemBound(
    OodleLZ_Compressor compressor,
    OodleLZ_CompressionLevel level,
    OO_SINTa rawLen,
    const OodleLZ_CompressOptions * pOptions OODEFAULT(NULL)
    );
/* Return the maximum amount of scratch mem that will be needed by OodleLZ_Compress

    $:compressor    which OodleLZ variant to use in compression
    $:level         OodleLZ_CompressionLevel controls how much CPU effort is put into maximizing compression
    $:rawLen        maximum number of bytes you will compress (plus dictionary backup)
    $:pOptions      (optional) options; if NULL, $OodleLZ_CompressOptions_GetDefault is used

    If you pass scratch mem to $OodleLZ_Compress of this size, it is gauranteed to do no allocations.
    (normally if it runs out of scratch mem, it falls back to the installed allocator)

    For _rawLen_ pass at least the maximum size you will ever encode.  If your data is divided into chunks,
    pass the chunk size.  If you will encode full buffers of unbounded size, pass -1.

    The options must be the same as when you call $OodleLZ_Compress

    Some options and levels may not have simple finite bounds.  Then $OODLELZ_SCRATCH_MEM_NO_BOUND is returned
    and the call to $OodleLZ_Compress may use the allocator even if infinite scratch memory is provided.
    Currently this applies to all the Optimal levels.

    When OODLELZ_SCRATCH_MEM_NO_BOUND is returned, you can still pass in scratch mem which will be used before
    going to the plugin allocator.

*/

// get maximum expanded size for compBuf alloc :
//  (note this is actually larger than the maximum compressed stream, it includes trash padding)
OOFUNC1 OO_SINTa OOFUNC2 OodleLZ_GetCompressedBufferSizeNeeded(OodleLZ_Compressor compressor,OO_SINTa rawSize);
/* Return the size you must malloc the compressed buffer

    $:compressor    compressor used; OodleLZ_Compressor_Invalid to make it enough for any compressor
    $:rawSize       uncompressed size you will compress into this buffer

    The _compBuf_ passed to $OodleLZ_Compress must be allocated at least this big.

    note this is actually larger than the maximum size of a compressed stream, it includes overrun padding.

*/

// decBuf needs to be a little larger than rawLen,
//  this will tell you exactly how much :
OOFUNC1 OO_SINTa OOFUNC2 OodleLZ_GetDecodeBufferSize(OodleLZ_Compressor compressor,OO_SINTa rawSize,OO_BOOL corruptionPossible);
/* Get the size you must malloc the decode (raw) buffer

    $:compressor    compressor used; OodleLZ_Compressor_Invalid to make it enough for any compressor
    $:rawSize       uncompressed (raw) size without padding
    $:corruptionPossible    true if it is possible for the decoder to get corrupted data
    $:return        size of buffer to malloc; slightly larger than rawSize if padding is needed

    As of Oodle 2.9.0 this function is deprecated.  For all new codecs you can just use the size of the
    uncompressed data for the decode buffer size (_rawSize_), no padding is needed.
    
    Note that LZB16 is still supported in 2.9.0 but does require padding when used in a circular
    window (which is deprecated).

    This padding is necessary for the older compressors when FuzzSafe_No is used.  The old compressors
    and FuzzSafe_No are no longer supported.

    If _corruptionPossible_ is true, a slightly larger buffer size is returned.

    If _corruptionPossible_ is false, then you must ensure that the decoder does not get corrupted data,
    either by passing $OodleLZ_CheckCRC_Yes , or by your own mechanism.

    Note about possible overrun in LZ decoding (applies to the old non-fuzz-safe compressors) : 
    as long as the compresseddata is not corrupted,
    and you decode either the entire compressed buffer, or an integer number of "seek chunks" ($OODLELZ_BLOCK_LEN),
    then there will be no overrun.  So you can decode LZ data in place and it won't stomp any following bytes.
    If those conditions are not true (eg. decoding only part of a larger compressed stream, decoding
    around a circular window, decoding data that may be corrupted), then there may be some limited amount of
    overrun on decode, as returned by $OodleLZ_GetDecodeBufferSize.
    

*/

// OodleLZ_GetInPlaceDecodeBufferSize :
//  after compressing, ask how big the in-place buffer needs to be
OOFUNC1 OO_SINTa OOFUNC2 OodleLZ_GetInPlaceDecodeBufferSize(OodleLZ_Compressor compressor,OO_SINTa compLen, OO_SINTa rawLen);
/* Get the size of buffer needed for "in place" decode

    $:compressor    compressor used; OodleLZ_Compressor_Invalid to make it enough for any compressor
    $:compLen       compressed data length
    $:rawLen        decompressed data length
    $:return        size of buffer needed for "in place" decode ; slighly larger than rawLen

    To do an "in place" decode, allocate a buffer of this size (or larger).  Read the compressed data into the end of
    the buffer, and decompress to the front of the buffer.  The size returned here guarantees that the writes to the
    front of the buffer don't conflict with the reads from the end.

    If _compressor_ is one of the new codecs (Kraken,Mermaid,Selkie,Leviathan), the padding for in place decodes can be
    very small indeed.  It is assumed you will be passing FuzzSafe_Yes to the decompress call.

    If _compLen_ is unknown, you want an in place buffer size that can accomodate any compressed data, then
    pass compLen = 0.

    See $OodleLZ_Decompress for more.
*/

// GetCompressedStepForRawStep is at OODLELZ_QUANTUM_LEN granularity
//  returns how many packed bytes to step to get the desired raw count step
OOFUNC1 OO_SINTa OOFUNC2 OodleLZ_GetCompressedStepForRawStep(
                                    const void * compPtr, OO_SINTa compAvail,
                                    OO_SINTa startRawPos, OO_SINTa rawSeekBytes,
                                    OO_SINTa * pEndRawPos OODEFAULT(NULL),
                                    OO_BOOL * pIndependent OODEFAULT(NULL) );
/* How many bytes to step a compressed pointer to advance a certain uncompressed amount

    $:compPtr   current compressed pointer
    $:compAvail compressed bytes available at compPtr
    $:startRawPos   initial raw pos (corresponding to compPtr)
    $:rawSeekBytes  the desired step in raw bytes, must be a multiple of $OODLELZ_QUANTUM_LEN or $OODLELZ_BLOCK_LEN
    $:pEndRawPos    (optional) filled with the end raw pos actually reached
    $:pIndependent  (optional) filled with a bool that is true if the current chunk is independent from previous
    $:return        the number of compressed bytes to step

    You should try to use GetCompressedStepForRawStep only at block granularity - both _startRawPos_ and
    _rawSeekBytes_ should be multiples of OODLELZ_BLOCK_LEN (except at the end of the stream).  As long as you
    do that, then *pEndRawPos will = startRawPos + rawSeekBytes.

    You can use it at quantum granularity (OODLELZ_QUANTUM_LEN), but there are some caveats.  You cannot step
    quanta inside uncompressed blocks, only in normal LZ blocks.  If you try to seek quanta inside an uncompressed
    block, you will get *pEndRawPos = the end of the block.

    You can only resume seeking from *pEndRawPos .

    returns 0 for valid not-enough-data case
    returns -1 for error

    If _compAvail_ is not the whole compressed buffer, then the returned step may be less than the amount you requested.
    eg. if the compressed data in _compAvail_ does not contain enough data to make a step of _rawSeekBytes_ a smaller
    step will be taken.
    NOTE : *can* return comp step > comp avail !


*/

OOFUNC1 OodleLZ_Compressor OOFUNC2 OodleLZ_GetAllChunksCompressor(const void * compBuf,OO_SINTa compBufSize,
                                    OO_SINTa rawLen);
/* ask who compressed all chunks in this buf chunk

    $:compBuf       pointer to compressed data; must be the start of compressed buffer, or a step of $OODLELZ_BLOCK_LEN raw bytes
    $:compBufSize   size of _compBuf_
    $:rawLen        rawlen of data in _compBuf_
    $:return        the $OodleLZ_Compressor used to encode this chunk

    returns a simple compressor (for example OodleLZ_Compressor_Kraken) if that was used on all chunks

    returns OodleLZ_Compressor_Hydra if different NewLZ encoders were used (for example Kraken+Mermaid)

    returns OodleLZ_Compressor_Count if a heterogenous mix of compressors was used (not just NewLZ)

    returns OodleLZ_Compressor_Invalid on error

    note this is only for this chunk - later chunks may have different compressors (eg. with Hydra)
    if you compressed all chunks the same it's up to you to store that info in your header

    returns OodleLZ_Compressor_Invalid if _compBufSize_ is too small or any chunk is corrupt
*/

OOFUNC1 OodleLZ_Compressor OOFUNC2 OodleLZ_GetFirstChunkCompressor(const void * compChunkPtr,
                                    OO_SINTa compBufAvail,
                                    OO_BOOL * pIndependent);
/* ask who compressed this chunk

    $:compChunkPtr  pointer to compressed data; must be the start of compressed buffer, or a step of $OODLELZ_BLOCK_LEN raw bytes
    $:compBufAvail  number of bytes at _compChunkPtr_ available to read
    $:pIndependent  (optional) filled with a bool for whether this chunk is independent of predecessors
    $:return        the $OodleLZ_Compressor used to encode this chunk

    note this is only for this chunk - later chunks may have different compressors (eg. with Hydra)
    if you compressed all chunks the same it's up to you to store that info in your header

    Use $OodleLZ_GetAllChunksCompressor for data that might be mixed compressors.

    This replaces the deprecated function $OodleLZ_GetChunkCompressor

    returns OodleLZ_Compressor_Invalid if _compBufAvail_ is too small or the chunk is corrupt
*/

OOFUNC1 OodleLZ_Compressor OOFUNC2 OodleLZ_GetChunkCompressor(const void * compChunkPtr,
                                    OO_SINTa compBufAvail,
                                    OO_BOOL * pIndependent);
/* Deprecated entry point for backwards compatibility

    Use $OodleLZ_GetFirstChunkCompressor or $OodleLZ_GetAllChunksCompressor

*/

//=======================================================

#define OODLE_HEADER_VERSION        ((46<<24)|(OODLE2_VERSION_MAJOR<<16)|(OODLE2_VERSION_MINOR<<8)|(OO_U32)sizeof(OodleLZ_SeekTable))       /*  OODLE_HEADER_VERSION is used to ensure the Oodle header matches the lib.  Don't copy the value of this macro, it will change when
    the header is rev'ed.

    This is what you pass to $OodleX_Init or $Oodle_CheckVersion
*/

OOFUNC1 OO_BOOL OOFUNC2 Oodle_CheckVersion(OO_U32 oodle_header_version, OO_U32 * pOodleLibVersion OODEFAULT(NULL));
/* Check the Oodle lib version against the header you are compiling with

    $:oodle_header_version  pass $OODLE_HEADER_VERSION here
    $:pOodleLibVersion      (optional) filled with the Oodle lib version
    $:return                false if $OODLE_HEADER_VERSION is not compatible with this lib
    
    If you use the Oodle2 Ext lib,, $OodleX_Init does it for you.  But if you want to check that you have a
    compatible lib before trying to Init, then use this.
*/

OOFUNC1 void OOFUNC2 Oodle_LogHeader(void);
/* Log the Oodle version & copyright

    Uses the log set with $OodleCore_Plugins_SetPrintf
*/

// define old names so they still compile :
#define OODLECORE_PLUGIN_JOB_MAX_DEPENDENCIES OODLE_JOB_MAX_DEPENDENCIES
#define t_fp_OodleCore_Plugin_Job t_fp_Oodle_Job

#ifdef _MSC_VER
#pragma warning(pop)
#pragma pack(pop, Oodle)
#endif

#endif // __OODLE2_H_INCLUDED__
