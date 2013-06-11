#ifndef	_ORANGES_FS_H_
#define	_ORANGES_FS_H_

#include "type.h"
#include "string.h"
#include "hd.h"
#include "proc.h"
#include "const.h"
#include "lib/fcntl.h"
#include "lib/stdio.h"
#define	INVALID_INODE		0
#define	ROOT_INODE		1

#define NO_DEV		0
#define DEV_FLOPPY  1
#define DEV_CDROM	2
#define DEV_HD		3
#define DEV_CHAR_TTY 4
#define DEV_SCSI    5

#define MAJOR_SHIFT 8
#define MAKE_DEV(a,b)  ( (a<<MAJOR_SHIFT) | b )
#define MAJOR(a)   ( (a>>MAJOR_SHIFT) & 0xff )
#define MINOR(a)   ( a & 0xff )

#define	MAGIC_V1	0x111
#define	SUPER_BLOCK_SIZE	56
#define	INODE_SIZE	32
#define MAX_FILENAME_LEN 12

#define K_MEMORY_SIZE	0x100000
#define K_MEMORY_SECTS  (K_MEMORY_SIZE/SECTOR_SIZE)
#define K_MEMORY_BEGIN  0x200000

#define	NR_FILE_DESC	64	/* FIXME */
#define NR_BLOCK_TABLE_ENTRY  7
#define ZONE_DATA_SIZE   0x100000  // 1MB
#define ZONE_DATA_SECTS  ( (ZONE_DATA_SIZE-1)/SECTOR_SIZE +1 )
#define ZONE_IMAP_SECTS ( (ZONE_DATA_SECTS-1)/SECTOR_SIZE +1 )
#define ZONE_METE_SECTS	( ZONE_IMAP_SECTS + (ZONE_DATA_SECTS * 64 -1 )/SECTOR_SIZE +1)
#define ZONE_ALL_SECTS (ZONE_DATA_SECTS + ZONE_METE_SECTS)

#define FIRST_SUPER_BLOCK 2
#define FIRST_BLOCK_TABLE 4
#define FIRST_ZONE		  6

/* INODE::i_mode (octal, lower 32 bits reserved) */
#define I_TYPE_MASK     0170000
#define I_REGULAR       0100000
#define I_BLOCK_SPECIAL 0060000
#define I_DIRECTORY     0040000
#define I_CHAR_SPECIAL  0020000
#define I_NAMED_PIPE	0010000

#define MAX_PATH  256

struct dev_drv_map {
	int driver_nr; /**< The proc nr.\ of the device driver. */
};

struct buffer_head{
	void *pdata;
	u64	 blocknr;
	u16  dev;
	u8	 size;
	u8   dirt;
	u8   count;
	u8   lock;
	struct proc *wait;
	struct buffer_head *prev;
	struct buffer_head *next;
	struct buffer_head *prev_free;
	struct buffer_head *next_free;
};
//32
struct d_block_table_entry{
	u32 block_size; //1,2,4,...
	u32 first_inode_num;
	u32 free_inode_num;
	u32 inode_count;
	u32 start_imap_nr;
	u32 start_itable_nr;
	u32 start_data_nr;
	u32 dev;
};

struct d_super_block {
	u32	magic;		  /**< Magic number */
	u32	first_block_table_sect;	  /**< Number of the 1st data sector */
	u32 root_inode;
	u32 zone_mete_sects;
	u32 zone_data_sects;
};

struct m_super_block{
	u32 magic;
	u32 first_block_table_sect;
	u32 root_inode;
	u32 zone_mete_sects;
	u32 zone_data_sects;

	//following only in memory
	u16	sb_dev; 	/**< the super block's home device */
	u8  dirt;
	u64 size;
	struct d_block_table_entry p_block_table[NR_BLOCK_TABLE_ENTRY];
	struct buffer_head *imap[8];
};

//64字节
struct d_inode {
	u32	mode;		/**< Accsess mode. Unused currently */
	u32 size; //0,1,2,3...
	u32 start_data_sect;
	u32 next_inode_id;
	u32 uid;
	u32 gid;
	u64 mtime;
	u32 nlinks;
	u64 bitmap;
	char name[20];
};

struct m_inode{
	u32 mode;
	u16 size; //0,1,2,3,4,5,6
	u32 start_data_sect;
	u32 next_inode_id;
	u32 uid;
	u32 gid;
	u64 mtime;
	u32 nlinks;
	u64 bitmap;
	char name[20];
	/* the following items are only present in memory */
	u16 dev;
	u32 inode_num;
	u32 start_itable_sect;
	u32 start_imap_sect;
	u32 zone_first_inode_num;
	u8	count;
	u8  lock;
	u8  dirt;
	u8  update;
	u32 seek;
	u64 atime;
	u64 ctime;
};
struct d_name_value_pair{
	char *name;
	u32  value;
};
//8 byte
struct d_key_value_pair{
	u32 key;
	u32 value;
};
//512 byte
struct d_btree_node{
	u8  num;
	u8  isroot;
	u16 level;
	struct d_key_value_pair kv[63];
	u16 prev_node_index;
	u16 next_node_index;
};

struct file_desc {
	u32		fd_mode;	/**< R or W */
	u32		fd_flags;
	u32		fd_count;
	u32		fd_pos;		/**< Current position for R/W. */
	struct m_inode*	fd_inode;	/**< Ptr to the i-node */
};

extern struct file_desc file_desc_table[NR_FILE_DESC];
extern struct m_inode inode_table[NR_INODE];
extern struct m_super_block super_block_table[];
extern MESSAGE fs_msg;
extern struct proc *pcaller;
extern struct m_inode *root_inode;
extern u8 *fsbuf ;
extern struct dev_drv_map dd_map[];

extern void *kmalloc( int n_blocks );
extern void kfree( void* p );
extern void init_hash();
extern void init_buffer( u32 buffer_end );


extern void mount_root( void );
extern struct m_super_block *get_super_block( int dev );
extern struct d_block_table_entry *get_block_table( int dev, int size, int n );

extern int rw_sector( int io_type, struct buffer_head *bh );

extern struct buffer_head *bread( int dev, int block, int size );
extern int bwrite( struct buffer_head *bh );
extern void invalidate_inode( int dev );
extern void brelse( struct buffer_head *bh );
extern void bdirty( struct buffer_head *bh );

extern struct m_inode *get_inode( int dev, int nr );
extern void put_inode( struct m_inode *inode );
extern struct buffer_head *get_inode_data( int dev, int nr );
extern struct m_inode *new_inode( int dev, int size );
extern void mov_inode_data( struct m_inode *des, struct m_inode *src );

extern u32 hash_string( char *name, u8 hash_type );
extern int btree_find( struct m_inode *inode, char *name );
extern struct d_btree_node *btree_new_root( struct buffer_head *bh, struct d_name_value_pair *pkv, int num );
extern int btree_insert( struct m_inode *inode, const char *name, u32 inode_num );

extern int find_file_in_dir( char *name, int dev, int dir_inode );
extern int find_entry( char *path_name, char **file_name, int *dir_inode_nr );

extern int do_open( MESSAGE *msg );
extern int do_close( MESSAGE *msg );
extern int do_rdwr( MESSAGE *msg );
extern int do_exec( MESSAGE *msg );

extern int getpid();
#define RD_SECT(bh) rw_sector(DEV_READ,bh) 
#define WR_SECT(bh) rw_sector(DEV_WRITE,bh) 

#endif /* _ORANGES_FS_H_ */
