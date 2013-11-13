/*
  BD translator - Exports Block devices on server side as regular
  files to client

  Copyright IBM, Corp. 2012

  This file is part of GlusterFS.

  Author:
  M. Mohan Kumar <mohan@in.ibm.com>

  This file is licensed to you under your choice of the GNU Lesser
  General Public License, version 3 or any later version (LGPLv3 or
  later), or the GNU General Public License, version 2 (GPLv2), in all
  cases as published by the Free Software Foundation.
*/

#ifndef _BD_H
#define _BD_H

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include "xlator.h"
#include "mem-types.h"

#define BD_XLATOR "block device mapper xlator"
#define BACKEND_VG "vg"
#define GF_XATTR "user.glusterfs"
#define BD_XATTR GF_XATTR ".bd"

#define BD_LV "lv"
#define BD_THIN "thin"

#define LVM_RESIZE "/sbin/lvresize"
#define LVM_CREATE "/sbin/lvcreate"

#define VOL_TYPE "volume.type"
#define VOL_CAPS "volume.caps"

#define ALIGN_SIZE 4096

#define BD_CAPS_BD               0x01
#define BD_CAPS_THIN             0x02

#define BD_VALIDATE_MEM_ALLOC(buff, op_errno, label)                \
        if (!buff) {                                                \
                op_errno = ENOMEM;                                  \
                gf_log (this->name, GF_LOG_ERROR, "out of memory"); \
                goto label;                                         \
        }

#define BD_VALIDATE_LOCAL_OR_GOTO(local, op_errno, label) \
        if (!local) {                                     \
                op_errno = EINVAL;                        \
                goto label;                               \
        }

#define BD_STACK_UNWIND(typ, frame, args ...) do {      \
        bd_local_t *__local = frame->local;             \
        xlator_t   *__this = frame->this;               \
                                                        \
        frame->local = NULL;                            \
        STACK_UNWIND_STRICT (typ, frame, args);         \
        if (__local)                                    \
                bd_local_free (__this, __local);        \
        } while (0)

typedef char bd_gfid_t[GF_UUID_BUF_SIZE];

enum gf_bd_mem_types_ {
        gf_bd_private  = gf_common_mt_end + 1,
        gf_bd_attr,
        gf_bd_fd,
        gf_bd_mt_end
};

/**
 * bd_fd - internal structure
 */
typedef struct bd_fd {
        int             fd;
        int32_t         flag;
} bd_fd_t;

typedef struct bd_priv {
        lvm_t             handle;
        char              *vg;
        char              *pool;
        int                caps;
} bd_priv_t;


typedef enum bd_type {
        BD_TYPE_NONE,
        BD_TYPE_LV,
} bd_type_t;

typedef struct {
        struct iatt  iatt;
        char        *type;
} bd_attr_t;

typedef struct {
        dict_t      *dict;
        bd_attr_t   *bdatt;
        inode_t     *inode;
        loc_t        loc;
        fd_t        *fd;
        data_t      *data; /* for setxattr */
} bd_local_t;

typedef struct {
        char            *lv;
        struct list_head list;
} bd_del_entry;

/* Prototypes */
int bd_inode_ctx_set (inode_t *inode, xlator_t *this, bd_attr_t *ctx);
int bd_inode_ctx_get (inode_t *inode, xlator_t *this, bd_attr_t **ctx);
int bd_scan_vg (xlator_t *this, bd_priv_t *priv);
bd_local_t *bd_local_init (call_frame_t *frame, xlator_t *this);
void bd_local_free (xlator_t *this, bd_local_t *local);
int bd_fd_ctx_get (xlator_t *this, fd_t *fd, bd_fd_t **bdfd);
char *page_aligned_alloc (size_t size, char **aligned_buf);
int bd_validate_bd_xattr (xlator_t *this, char *bd, char **type,
                          uint64_t *lv_size, uuid_t uuid);
uint64_t bd_get_default_extent (bd_priv_t *priv);
uint64_t bd_adjust_size (bd_priv_t *priv, uint64_t size);
int bd_create (uuid_t uuid, uint64_t size, char *type, bd_priv_t *priv);
int bd_resize (bd_priv_t *priv, uuid_t uuid, off_t size);
int bd_delete_lv (bd_priv_t *priv, const char *lv_name, int *op_errno);
int bd_snapshot_create (bd_local_t *local, bd_priv_t *priv);
int bd_clone (bd_local_t *local, bd_priv_t *priv);

int bd_merge (bd_priv_t *priv, uuid_t gfid);
int bd_get_origin (bd_priv_t *priv, loc_t *loc, fd_t *fd, dict_t *dict);
#endif