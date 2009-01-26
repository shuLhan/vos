SRC_D		= .
SRC_OP_D	= ${SRC_D}/op
SRC_PROC_D	= ${SRC_D}/proc

BUILD_D		= ../build
BUILD_OP_D	= ${BUILD_D}/op
BUILD_PROC_D	= ${BUILD_D}/proc

CC		= gcc
CFLAGS		= -Wall -O2 -I${SRC_D}
CFLAGS_DEBUG	= -Wall -g -O0 -I${SRC_D}
LDFLAGS		= -lpthread

COMPILE		= echo "[COMPILE] $@"; \
		${CC} ${CFLAGS} -c $< -o $@
BUILD		= echo "[_BUILD_] $@"; \
		${CC} ${CFLAGS} ${LDFLAGS} $^ -o $@

BIN		= ${BUILD_D}/vos
VOS_OBJS	=	\
			${BUILD_D}/vos_errno.o			\
			${BUILD_OP_D}/vos_String.o		\
			${BUILD_OP_D}/vos_File.o		\
			${BUILD_OP_D}/vos_LL.o			\
			${BUILD_OP_D}/vos_Filter.o		\
			${BUILD_OP_D}/vos_Field.o		\
			${BUILD_OP_D}/vos_Record.o		\
			${BUILD_OP_D}/vos_Bucket.o		\
			${BUILD_OP_D}/vos_StmtMeta.o		\
			${BUILD_OP_D}/vos_Stmt.o		\
			${BUILD_OP_D}/vos_StmtSet.o		\
			${BUILD_OP_D}/vos_StmtLoad.o		\
			${BUILD_OP_D}/vos_StmtCreate.o		\
			${BUILD_OP_D}/vos_StmtSort.o		\
			${BUILD_OP_D}/vos_StmtJoin.o		\
			${BUILD_PROC_D}/vos_parser.o		\
			${BUILD_PROC_D}/vos_set.o		\
			${BUILD_PROC_D}/vos_load.o		\
			${BUILD_PROC_D}/vos_sort_merge.o	\
			${BUILD_PROC_D}/vos_sort.o		\
			${BUILD_PROC_D}/vos_create.o		\
			${BUILD_PROC_D}/vos_join.o		\
			${BUILD_D}/vos.o

all: clean prebuild ${BIN}

debug: CFLAGS = ${CFLAGS_DEBUG}
debug: clean prebuild ${BIN}

prebuild:
	@mkdir -p ${BUILD_D}
	@mkdir -p ${BUILD_OP_D}
	@mkdir -p ${BUILD_PROC_D}

${BIN}: ${VOS_OBJS}
	@${BUILD}

${BUILD_D}/%.o: ${SRC_D}/%.c ${SRC_D}/%.h
	@${COMPILE}

${BUILD_OP_D}/%.o: ${SRC_OP_D}/%.c ${SRC_OP_D}/%.h
	@${COMPILE}

${BUILD_PROC_D}/%.o: ${SRC_PROC_D}/%.c ${SRC_PROC_D}/%.h
	@${COMPILE}
clean:
	@echo "[_CLEAN_]"
	@rm -f ${BIN} ${VOS_OBJS} valgrind.out *.tmp *.sort data.output
