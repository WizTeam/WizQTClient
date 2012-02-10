create table WIZ_DOCUMENT_ATTACHMENT
(
   ATTACHMENT_GUID                   char(36)                       not null,
   DOCUMENT_GUID                     varchar(36)                    not null,
   ATTACHMENT_NAME                   varchar(768)                   not null,
   ATTACHMENT_URL                    varchar(2048),
   ATTACHMENT_DESCRIPTION            varchar(600),
   DT_INFO_MODIFIED                  char(19),
   ATTACHMENT_INFO_MD5               char(32),
   DT_DATA_MODIFIED                  char(19),
   ATTACHMENT_DATA_MD5               char(32),
   WIZ_VERSION                       int64,
   primary key (ATTACHMENT_GUID)
)