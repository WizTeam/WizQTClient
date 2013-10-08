create table WIZ_DOCUMENT_TAG
(
   DOCUMENT_GUID                  char(36)                       not null,
   TAG_GUID                       char(36)                       not null,
   primary key (DOCUMENT_GUID, TAG_GUID)
)