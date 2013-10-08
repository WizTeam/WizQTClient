create table WIZ_TAG_GROUP
(
   TAG_GROUP_GUID                 char(36)                       not null,
   TAG_GROUP_NAME                 varchar(150),
   TAG_GROUP_DESCRIPTION          varchar(600),
   DT_MODIFIED                    char(19),
   WIZ_VERSION                    int64,
   primary key (TAG_GROUP_GUID)
)
