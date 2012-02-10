create table WIZ_OBJECT_EX
(
   OBJECT_GUID                  char(36)                       not null,
   OBJECT_TYPE                  char(20)                       not null,
   OBJECT_RESERVED1		int,
   OBJECT_RESERVED2		int,
   OBJECT_RESERVED3		int,
   OBJECT_RESERVED4		int,
   OBJECT_RESERVED5		varchar(200),
   OBJECT_RESERVED6		varchar(500),
   OBJECT_RESERVED7		varchar(1000),
   OBJECT_RESERVED8		varchar(5000),

   primary key (OBJECT_GUID, OBJECT_TYPE)
)
