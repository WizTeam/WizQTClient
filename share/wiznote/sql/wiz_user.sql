CREATE TABLE [WIZ_USER] (
  [BIZ_GUID] CHAR(36) NOT NULL,
  [USER_ID] varchar(128) NOT NULL, 
  [USER_GUID] varchar(36), 
  [USER_ALIAS] varchar(32),
  [USER_PINYIN] varchar(192),
   primary key ([BIZ_GUID], [USER_ID])
);

