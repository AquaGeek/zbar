#if !defined(_qrcode_H)
# define _qrcode_H (1)

# if defined(__cplusplus)
extern "C" {
# endif

typedef struct qr_reader          qr_reader;

typedef struct qr_code_data_entry qr_code_data_entry;
typedef struct qr_code_data       qr_code_data;
typedef struct qr_code_data_list  qr_code_data_list;

typedef enum qr_mode{
  /*Numeric digits ('0'...'9').*/
  QR_MODE_NUM=1,
  /*Alphanumeric characters ('0'...'9', 'A'...'Z', plus the punctuation
     ' ', '$', '%', '*', '+', '-', '.', '/', ':').*/
  QR_MODE_ALNUM,
  /*Structured-append header.*/
  QR_MODE_STRUCT,
  /*Raw 8-bit bytes.*/
  QR_MODE_BYTE,
  /*FNC1 marker (for more info, see http://www.mecsw.com/specs/uccean128.html).
    In the "first position" data is formatted in accordance with GS1 General
     Specifications.*/
  QR_MODE_FNC1_1ST,
  /*Mode 6 reserved?*/
  /*Extended Channel Interpretation code.*/
  QR_MODE_ECI=7,
  /*SJIS kanji characters.*/
  QR_MODE_KANJI,
  /*FNC1 marker (for more info, see http://www.mecsw.com/specs/uccean128.html).
    In the "second position" data is formatted in accordance with an industry
     application as specified by AIM Inc.*/
  QR_MODE_FNC1_2ND
}qr_mode;

/*Check if a mode has a data buffer associated with it.
  Currently this is only modes with exactly one bit set.*/
#define QR_MODE_HAS_DATA(_mode) (!((_mode)&(_mode)-1))

/*ECI may be used to signal a character encode for the data.*/
typedef enum qr_eci_encoding{
  /*GLI0 is like CP437, but the encoding is reset at the beginning of each
     structured append symbol.*/
  QR_ECI_GLI0,
  /*GLI1 is like ISO8859_1, but the encoding is reset at the beginning of each
     structured append symbol.*/
  QR_ECI_GLI1,
  /*The remaining encodings do not reset at next structured append symbol.*/
  QR_ECI_CP437,
  /*Western European.*/
  QR_ECI_ISO8859_1,
  /*Central European.*/
  QR_ECI_ISO8859_2,
  /*South European.*/
  QR_ECI_ISO8859_3,
  /*North European.*/
  QR_ECI_ISO8859_4,
  /*Cyrillic.*/
  QR_ECI_ISO8859_5,
  /*Arabic.*/
  QR_ECI_ISO8859_6,
  /*Greek.*/
  QR_ECI_ISO8859_7,
  /*Hebrew.*/
  QR_ECI_ISO8859_8,
  /*Turkish.*/
  QR_ECI_ISO8859_9,
  /*Nordic.*/
  QR_ECI_ISO8859_10,
  /*Thai.*/
  QR_ECI_ISO8859_11,
  /*There is no ISO/IEC 8859-12.*/
  /*Baltic rim.*/
  QR_ECI_ISO8859_13=QR_ECI_ISO8859_11+2,
  /*Celtic.*/
  QR_ECI_ISO8859_14,
  /*Western European with euro.*/
  QR_ECI_ISO8859_15,
  /*South-Eastern European (with euro).*/
  QR_ECI_ISO8859_16,
  /*ECI 000019 is reserved?*/
  /*Shift-JIS.*/
  QR_ECI_SJIS=20
}qr_eci_encoding;


/*A single unit of parsed QR code data.*/
struct qr_code_data_entry{
  /*The mode of this data block.*/
  qr_mode mode;
  union{
    /*Data buffer for modes that have one.*/
    struct{
      unsigned char *buf;
      int            len;
    }data;
    /*Decoded "Extended Channel Interpretation" data.*/
    unsigned eci;
    /*Structured-append header data.*/
    struct{
      unsigned char sa_size;
      unsigned char sa_index;
      unsigned char sa_parity;
    }sa;
  }payload;
};



/*Low-level QR code data.*/
struct qr_code_data{
  /*The decoded data entries.*/
  qr_code_data_entry *entries;
  int                 nentries;
  /*The code version (1...40).*/
  unsigned char       version;
  /*The ECC level (0...3, corresponding to 'L', 'M', 'Q', and 'H').*/
  unsigned char       ecc_level;
  /*Structured-append information.*/
  /*The size of the structured-append group, or 0 if there was no S-A header.*/
  unsigned char       sa_size;
  /*The index of this code in the structured-append group.
    If sa_size is zero, this is undefined.*/
  unsigned char       sa_index;
  /*The parity of the entire structured-append group.
    If sa_size is zero, this is undefined.*/
  unsigned char       sa_parity;
  /*The parity of this code.
    If sa_size is zero, this is undefined.*/
  unsigned char       self_parity;
};


struct qr_code_data_list{
  qr_code_data *qrdata;
  int           nqrdata;
  int           cqrdata;
};

/*Initialize an empty list of qrcode data.*/
void qr_code_data_list_init(qr_code_data_list *_qrlist);
/*Free a list of decoded QR code data returned by qr_reader_locate().*/
void qr_code_data_list_clear(qr_code_data_list *_qrlist);

/*Extract text from a list of QR codes.
  All text is returned in UTF-8.
  If _allow_partial_sa is non-zero, then any structured-append group that does
   not have all of its members is decoded as separate strings for each
   contiguous group of codes which are present; otherwise the whole group is
   ignored.
  Note that isolated members of a structured-append group may be decoded with
   the wrong character set, since the correct setting cannot be propagated
   between codes.
  Return: The number of strings which were successfully extracted from the
   codes; this will be at most the number of codes.*/
int qr_code_data_list_extract_text(const qr_code_data_list *_qrlist,
 char ***_text,int _allow_partial_sa);



qr_reader *qr_reader_alloc(void);
void qr_reader_free(qr_reader *_reader);

/*Locate all the QR codes in the given binary image.*/
int qr_reader_locate(qr_reader *_reader,qr_code_data_list *_qrlist,
 const unsigned char *_img,int _width,int _height);
/*Extract text from all QR codes in the given binary image.
  All text is returned in UTF-8.*/
int qr_reader_extract_text(qr_reader *_reader,const unsigned char *_img,
 int _width,int _height,char ***_text,int _allow_partial_sa);

void qr_text_list_free(char **_text,int _ntext);

# if defined(__cplusplus)
}
# endif

#endif
