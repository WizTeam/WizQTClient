//
//  DTWebResource.h
//  DTWebArchive
//
//  Created by Oliver Drobnik on 9/2/11.
//  Copyright 2011 Cocoanetics. All rights reserved.
//
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>


extern NSString * WebResourceDataKey;
extern NSString * WebResourceFrameNameKey;
extern NSString * WebResourceMIMETypeKey;
extern NSString * WebResourceURLKey;
extern NSString * WebResourceTextEncodingNameKey;
extern NSString * WebResourceResponseKey;

/** A `DTWebResource` object represents a downloaded URL. It encapsulates the data of the download as well as other resource properties such as the URL, MIME type, and frame name.

 Use the <initWithData:URL:MIMEType:textEncodingName:frameName:> method to initialize a newly created `DTWebResource` object. Use the other methods in this class to get the properties of a `DTWebResource` object.
 */
@interface DTWebResource : NSObject <NSKeyedUnarchiverDelegate>
{
    NSData *_data;
    NSString *_frameName;
    NSString *_MIMEType;
    NSURL *_URL;
    NSString *_textEncodingName;
}

/**---------------------------------------------------------------------------------------
 * @name Initializing
 * ---------------------------------------------------------------------------------------
 */

/** Initializes and returns a web resource instance.

 @param data The download data.
 @param URL The download URL.
 @param MIMEType The MIME type of the data.
 @param textEncodingName The IANA encoding name (for example, “utf-8” or “utf-16”). This parameter may be `nil`.
 @param frameName The name of the frame. Use this parameter if the resource represents the contents of an entire HTML frame; otherwise pass `nil`.
 @return An initialized web resource.
 */
- (id)initWithData:(NSData *)data URL:(NSURL *)URL MIMEType:(NSString *)MIMEType textEncodingName:(NSString *)textEncodingName frameName:(NSString *)frameName;

//- (UIImage *)image;

/**---------------------------------------------------------------------------------------
 * @name Getting Attributes
 * ---------------------------------------------------------------------------------------
 */

/** Returns the receiver’s data.

 @return The download data.
 */
- (NSData *)data;

/** Returns the receiver’s frame name.

@return The name of the frame. If the receiver does not represent the contents of an entire HTML frame, this method returns `nil`.
 */
- (NSString *)frameName;

/** Returns the receiver’s MIME type.

 @return The MIME type of the data.
 */
- (NSString *)MIMEType;

/** Returns the receiver’s URL.

 @return The download URL.
 */
- (NSURL *)URL;

/** Returns the receiver’s text encoding name.

 @return The IANA encoding name (for example, “utf-8” or “utf-16”), or `nil` if the name does not exist.
 */
- (NSString *)textEncodingName;

@end



/** Private interface to work with NSDictionary */
@interface DTWebResource (Dictionary)

- (id)initWithDictionary:(NSDictionary *)dictionary;
- (NSDictionary *)dictionaryRepresentation;

@end
//
//  DTWebArchive.h
//  DTWebArchive
//
//  Created by Oliver Drobnik on 9/2/11.
//  Copyright 2011 Cocoanetics. All rights reserved.
//


/** The pasteboard type for this class. */
extern NSString * WebArchivePboardType;

/** A `DTWebArchive` object represents a webpage that can be archived—for example, archived on disk or on the pasteboard. A `DTWebArchive` object contains the main resource, as well as the subresources and subframes of the main resource. The main resource can be an entire webpage, a portion of a webpage, or some other kind of data such as an image. Use this class to archive webpages, or place a portion of a webpage on the pasteboard, or to represent rich web content in any application.
 */

@interface DTWebArchive : NSObject
{
    DTWebResource *_mainResource;
    NSArray *_subresources;
    NSArray *_subframeArchives;
}

/**---------------------------------------------------------------------------------------
 * @name Initializing
 * ---------------------------------------------------------------------------------------
 */

/** Initializes and returns the receiver.

 Use the <data> method to get the receiver’s data.
 @param data The initial content data.
 */
- (id)initWithData:(NSData *)data;

/** Initializes the receiver with a resource and optional subresources and subframe archives.

This method initializes and returns the receiver.

 @param mainResource The main resource for the archive.
 @param subresources An array of <DTWebResource> objects or `nil` if none are specified.
 @param subframeArchives An array of <DTWebArchive> objects used by the sub frames or `nil` if none are specified.
 */
- (id)initWithMainResource:(DTWebResource *)mainResource subresources:(NSArray *)subresources subframeArchives:(NSArray *)subframeArchives;

/**---------------------------------------------------------------------------------------
 * @name Getting Attributes
 * ---------------------------------------------------------------------------------------
 */

/** Returns the receiver’s main resource. */
- (DTWebResource *)mainResource;

/** Returns the receiver’s sub resources, or `nil` if there are none. */
- (NSArray *)subresources;

/** Returns archives representing the receiver’s sub frame archives or `nil` if there are none. */
- (NSArray *)subframeArchives;

/** Returns the data representation of the receiver.

 The data returned can be used to save the web archive to a file, to put it on the pasteboard using the `WebArchivePboardType` type, or used to initialize another web archive using the <initWithData:> method.
 */
- (NSData *)data;

@end

//
//  DTWebResource.m
//  DTWebArchive
//
//  Created by Oliver Drobnik on 9/2/11.
//  Copyright 2011 Cocoanetics. All rights reserved.
//


NSString *WebResourceDataKey = @"WebResourceData";
NSString *WebResourceFrameNameKey = @"WebResourceFrameName";
NSString *WebResourceMIMETypeKey = @"WebResourceMIMEType";
NSString *WebResourceURLKey = @"WebResourceURL";
NSString *WebResourceTextEncodingNameKey =  @"WebResourceTextEncodingName";
NSString *WebResourceResponseKey = @"WebResourceResponse";

@interface DTWebResource ()

@property(nonatomic, retain, readwrite) NSData *data;
@property(nonatomic, retain, readwrite) NSString *frameName;
@property(nonatomic, retain, readwrite) NSString *MIMEType;
@property(nonatomic, retain, readwrite) NSURL *URL;
@property(nonatomic, retain, readwrite) NSString *textEncodingName;

@end


@implementation DTWebResource

- (id)initWithData:(NSData *)data URL:(NSURL *)URL MIMEType:(NSString *)MIMEType textEncodingName:(NSString *)textEncodingName frameName:(NSString *)frameName
{
    self = [super init];

    if (self)
    {
        self.data = data;
        self.URL = URL;
        self.MIMEType = MIMEType;
        self.textEncodingName = textEncodingName;
        self.frameName = frameName;
    }

    return self;
}


#pragma mark Properties

@synthesize data = _data;
@synthesize frameName = _frameName;
@synthesize MIMEType = _MIMEType;
@synthesize URL = _URL;
@synthesize textEncodingName = _textEncodingName;

@end


@implementation DTWebResource (Dictionary)

- (id)initWithDictionary:(NSDictionary *)dictionary;
{
    NSData *data = nil;
    NSString *frameName = nil;
    NSString *mimeType = nil;
    NSURL *url = nil;
    NSString *textEncodingName = nil;

    data = [dictionary objectForKey:WebResourceDataKey];
    frameName = [dictionary objectForKey:WebResourceFrameNameKey];
    mimeType = [dictionary objectForKey:WebResourceMIMETypeKey];
    url = [NSURL URLWithString:[dictionary objectForKey:WebResourceURLKey]];
    textEncodingName = [dictionary objectForKey:WebResourceTextEncodingNameKey];

    // if we wanted to, here's the decoded response
    //		NSData *data2 = [dictionary objectForKey:WebResourceResponseKey];
    //		if (data2)
    //		{
    //			NSKeyedUnarchiver *unarchiver = [[[NSKeyedUnarchiver alloc] initForReadingWithData:data2] autorelease];
    //			NSHTTPURLResponse *response = [unarchiver decodeObjectForKey:WebResourceResponseKey];
    //			NSLog(@"%@", [response allHeaderFields]);
    //		}

    return [self initWithData:data URL:url MIMEType:mimeType textEncodingName:textEncodingName frameName:frameName];
}

- (NSDictionary *)dictionaryRepresentation
{
    NSMutableDictionary *tmpDict = [NSMutableDictionary dictionary];

    if (_data)
    {
        [tmpDict setObject:_data forKey:WebResourceDataKey];
    }

    if (_frameName)
    {
        [tmpDict setObject:_frameName forKey:WebResourceFrameNameKey];
    }

    if (_MIMEType)
    {
        [tmpDict setObject:_MIMEType forKey:WebResourceMIMETypeKey];
    }

    if (_textEncodingName)
    {
        [tmpDict setObject:_textEncodingName forKey:WebResourceTextEncodingNameKey];
    }

    if (_URL)
    {
        [tmpDict setObject:[_URL absoluteString] forKey:WebResourceURLKey];
    }

    // ignoring the NSURLResponse for now

    return tmpDict;
}


@end



//
//  DTWebArchive.m
//  DTWebArchive
//
//  Created by Oliver Drobnik on 9/2/11.
//  Copyright 2011 Cocoanetics. All rights reserved.
//


static NSString * const LegacyWebArchiveMainResourceKey = @"WebMainResource";
static NSString * const LegacyWebArchiveSubresourcesKey = @"WebSubresources";
static NSString * const LegacyWebArchiveSubframeArchivesKey =@"WebSubframeArchives";
static NSString * const LegacyWebArchiveResourceDataKey = @"WebResourceData";
static NSString * const LegacyWebArchiveResourceFrameNameKey = @"WebResourceFrameName";
static NSString * const LegacyWebArchiveResourceMIMETypeKey = @"WebResourceMIMEType";
static NSString * const LegacyWebArchiveResourceURLKey = @"WebResourceURL";
static NSString * const LegacyWebArchiveResourceTextEncodingNameKey = @"WebResourceTextEncodingName";
static NSString * const LegacyWebArchiveResourceResponseKey = @"WebResourceResponse";
static NSString * const LegacyWebArchiveResourceResponseVersionKey = @"WebResourceResponseVersion";

NSString * WebArchivePboardType = @"Apple Web Archive pasteboard type";



@interface DTWebArchive ()

@property (nonatomic, retain, readwrite) DTWebResource *mainResource;
@property (nonatomic, retain, readwrite) NSArray *subresources;
@property (nonatomic, retain, readwrite) NSArray *subframeArchives;

@end


/** Private interface to work with dictionaries */
@interface DTWebArchive (Dictionary)

- (id)initWithDictionary:(NSDictionary *)dictionary;
- (NSDictionary *)dictionaryRepresentation;
- (void)updateFromDictionary:(NSDictionary *)dictionary;

@end


@implementation DTWebArchive

#pragma mark Initialization

- (id)initWithData:(NSData *)data
{
    self = [super init];
    if (self)
    {
        NSError *error;
        NSDictionary *dict = [NSPropertyListSerialization propertyListWithData:data
                                                                                            options:0
                                                                                             format:NULL
                                                                                              error:&error];

        if (!dict)
        {
            NSLog(@"%@", [error localizedDescription]);
            return nil;
        }

        [self updateFromDictionary:dict];
    }

    return self;
}

- (id)initWithMainResource:(DTWebResource *)mainResource subresources:(NSArray *)subresources subframeArchives:(NSArray *)subframeArchives
{
    self = [super init];

    if (self)
    {
        self.mainResource = mainResource;
        self.subresources = subresources;
        self.subframeArchives = subframeArchives;
    }

    return self;
}

#pragma mark Getting Attributes

@synthesize mainResource = _mainResource;
@synthesize subresources = _subresources;
@synthesize subframeArchives = _subframeArchives;

- (NSData *)data
{
    // need to make a data representation first
    NSDictionary *dict = [self dictionaryRepresentation];

    NSError *error;
    NSData *data = [NSPropertyListSerialization dataWithPropertyList:dict
                                                                                 format:NSPropertyListBinaryFormat_v1_0
                                                                                options:0
                                                                                  error:&error];

    if (!data)
    {
        NSLog(@"%@", error);
    }

    return data;
}

@end


@implementation DTWebArchive (Dictionary)

- (id)initWithDictionary:(NSDictionary *)dictionary
{
    self = [super init];

    if (self)
    {
        if (!dictionary)
        {
            return nil;
        }

        [self updateFromDictionary:dictionary];
    }

    return self;
}

- (void)updateFromDictionary:(NSDictionary *)dictionary
{
    self.mainResource = [[DTWebResource alloc] initWithDictionary:[dictionary objectForKey:LegacyWebArchiveMainResourceKey]];

    NSArray *subresources = [dictionary objectForKey:LegacyWebArchiveSubresourcesKey];
    if (subresources)
    {
        NSMutableArray *tmpArray = [NSMutableArray array];

        // convert to DTWebResources
        for (NSDictionary *oneResourceDict in subresources)
        {
            DTWebResource *oneResource = [[DTWebResource alloc] initWithDictionary:oneResourceDict];
            [tmpArray addObject:oneResource];
        }

        self.subresources = tmpArray;
    }

    NSArray *subframeArchives = [dictionary objectForKey:LegacyWebArchiveSubframeArchivesKey];
    if (subframeArchives)
    {
        NSMutableArray *tmpArray = [NSMutableArray array];

        // convert dictionaries to DTWebArchive objects
        for (NSDictionary *oneArchiveDict in subframeArchives)
        {
            DTWebArchive *oneArchive = [[DTWebArchive alloc] initWithDictionary:oneArchiveDict];
            [tmpArray addObject:oneArchive];
        }

        self.subframeArchives = tmpArray;
    }
}

- (NSDictionary *)dictionaryRepresentation
{
    NSMutableDictionary *tmpDict = [NSMutableDictionary dictionary];

    if (_mainResource)
    {
        [tmpDict setObject:[_mainResource dictionaryRepresentation] forKey:LegacyWebArchiveMainResourceKey];
    }

    if (_subresources)
    {
        NSMutableArray *tmpArray = [NSMutableArray array];

        for (DTWebResource *oneResource in _subresources)
        {
            [tmpArray addObject:[oneResource dictionaryRepresentation]];
        }

        [tmpDict setObject:tmpArray forKey:LegacyWebArchiveSubresourcesKey];
    }

    if (_subframeArchives)
    {
        NSMutableArray *tmpArray = [NSMutableArray array];

        for (DTWebArchive *oneArchive in _subframeArchives)
        {
            [tmpArray addObject:[oneArchive dictionaryRepresentation]];
        }

        [tmpDict setObject:tmpArray forKey:LegacyWebArchiveSubframeArchivesKey];
    }

    return tmpDict;
}

@end

void WizTestWebArchieve()
{
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    NSData* data = [pasteboard dataForType:WebArchivePboardType];
    if (!data)
        return;
    DTWebArchive* archive = [[DTWebArchive alloc] initWithData:data];
    if (!archive)
        return;
    //
    NSURL* url = archive.mainResource.URL;
    NSLog(@"%@", url);
}
