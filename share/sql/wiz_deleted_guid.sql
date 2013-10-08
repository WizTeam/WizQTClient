create table WIZ_DELETED_GUID
(
   DELETED_GUID                   char(36)                       not null,
   GUID_TYPE                      int                            not null,
   DT_DELETED                     char(19),
   primary key (DELETED_GUID)
)