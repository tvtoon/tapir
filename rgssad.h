#define RGSS_MAINHDR    12
#define RGSS1_MINISHDR   8
#define RGSS1_VALIDPACK 20
#define RGSS3_MINISHDR  16
#define RGSS3_VALIDPACK 38

typedef struct
{
 unsigned int pos;
 unsigned int files;
 unsigned int datakey;
 unsigned int filens;
 char filen[PATH_MAX + 1];
} rgssa_subhdr;

typedef struct
{
 char magic[7 + 1];
 unsigned char version;
 unsigned int key;
 rgssa_subhdr *suba;
} rgssa_hdr;

int copyofmy_mkpath( char *path, const size_t pathl, const mode_t defperm );
int rgssad_createhierarchy( const rgssa_subhdr *rgsssh, const size_t filec );
int rgssad_extractf( FILE *rgssarc, const rgssa_subhdr *rgsssh, long int curpos, const size_t maxbsize, const size_t filec );
size_t rgssad_difpara( size_t *finali, size_t *availi, const size_t arrayc );
size_t rgssad_difseqst( size_t *availi, const size_t *pathsa, const rgssa_subhdr *rgsssh, const size_t filec );
size_t rgssad_dirpos( size_t *pathsa, const rgssa_subhdr *rgsssh, const size_t filec );
size_t rgssad_getheadf( FILE *rgssf, rgssa_hdr *rgssh );
size_t rgssad1_countsubheadf( FILE *rgssf, const unsigned int key );
size_t rgssad1_getsubheadf( FILE *rgssf, rgssa_subhdr *rgsssh, const unsigned int key );
size_t rgssad1_getsubheadfv( FILE *rgssf, rgssa_subhdr *rgsssh, const unsigned int key );
size_t rgssad3_countsubheadf( FILE *rgssf, const unsigned int key );
size_t rgssad3_getsubheadf( FILE *rgssf, rgssa_subhdr *rgsssh, const unsigned int key );
size_t rgssad3_getsubheadfv( FILE *rgssf, rgssa_subhdr *rgsssh, const unsigned int key );
void rgssad_decriptestem2( unsigned char *bufmod, const unsigned char *buforg, const unsigned int files, unsigned int curpos, unsigned int datakey );
void rgssad_decriptestem3( unsigned char *bufmod, const unsigned char *buforg, const unsigned int files, unsigned int datakey );
void rgssad_decryptrealm( unsigned char *bufmod, const unsigned char *buforg, const unsigned int curpos, const unsigned int files, unsigned int datakey );
void rgssad_fixpath( char *filen, const size_t filens );
void rgssad_inihead( rgssa_hdr *rgssh );
void rgssad_inisubhead( rgssa_subhdr *rgsssh );
void rgssad_print( const rgssa_subhdr rgsssh );
