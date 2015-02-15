create table WIZ_TAG
(
   TAG_GUID                       char(36)                       not null,
   TAG_GROUP_GUID                 char(36),
   TAG_NAME                       varchar(150),
   TAG_DESCRIPTION                varchar(600),
   DT_MODIFIED                    char(19),
   WIZ_VERSION                    int64,
   TAG_POS	                      int64,
   primary key (TAG_GUID)
)
