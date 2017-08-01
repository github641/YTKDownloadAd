//
//  YTKRequest.m
//
//  Copyright (c) 2012-2016 YTKNetwork https://github.com/yuantiku
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#import "YTKNetworkConfig.h"
#import "YTKRequest.h"
#import "YTKNetworkPrivate.h"

#ifndef NSFoundationVersionNumber_iOS_8_0
#define NSFoundationVersionNumber_With_QoS_Available 1140.11
#else
#define NSFoundationVersionNumber_With_QoS_Available NSFoundationVersionNumber_iOS_8_0
#endif

/* lzy注170713：
 自定义错误的 错误域
 */
NSString *const YTKRequestCacheErrorDomain = @"com.yuantiku.request.caching";


/* lzy注170713：
 定义了一个优先级低的线程的串行队列，保证所有请求在这个线程中, 将用于将缓存写入文件时使用，保证不占用cpu主线程资源，即使多个请求并发，也会使用这个标识了统一符号的同一个线程，避免了使用过多子线程，同样占用过多资源。
 */
static dispatch_queue_t ytkrequest_cache_writing_queue() {
    static dispatch_queue_t queue;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        dispatch_queue_attr_t attr = DISPATCH_QUEUE_SERIAL;
        if (NSFoundationVersionNumber >= NSFoundationVersionNumber_With_QoS_Available) {
            attr = dispatch_queue_attr_make_with_qos_class(attr, QOS_CLASS_BACKGROUND, 0);
        }
        queue = dispatch_queue_create("com.yuantiku.ytkrequest.caching", attr);
    });

    return queue;
}

/* lzy注170713：
 缓存数据的元数据信息类
 */
@interface YTKCacheMetadata : NSObject<NSSecureCoding>
/* lzy注170713：
 遵守了NSSecureCoding的缓存的元数据类YTKCacheMetadata。NSSecureCoding有何用？
 NSCoding是iOS上把模型对象直接转变成一个文件，然后再把这个文件重新加载到内存里一种极其简单和方便的方式，它并不需要任何文件解析和序列化的逻辑。如果要把对象保存到一个数据文件中（假设这个对象实现了NSCoding协议），那么你可以像下面这样做：
 [NSKeyedArchiver archiveRootObject:xxx toFile:someFile];
 XXX *xxx = [NSKeyedUnarchiver unarchiveObjectWithFile:someFile];
 
 这样做对于编译进APP里的mainbundle中的资源（nib等）来说是可以的，但是使用NSCoding来读写用户数据文件的问题在于，把全部的类编码到一个文件里，也就间接地给了这个文件访问你APP里面实例类的权限，存在安全风险。苹果引入了基于NSCoding的NSSecureCoding。除了在解码时要同时指定key和要解码的对象的类，如果要求的类和从文件中解码出的对象的类不匹配，NSCoder会抛出异常，告诉你数据已经被篡改了。
 
 
 */

/* lzy注170713：
 api版本
 */
@property (nonatomic, assign) long long version;
/* lzy注170713：
 敏感数据字符串
 */
@property (nonatomic, strong) NSString *sensitiveDataString;
/* lzy注170713：
 字符串编码
 */
@property (nonatomic, assign) NSStringEncoding stringEncoding;
/* lzy注170713：
 创建日期
 */
@property (nonatomic, strong) NSDate *creationDate;
/* lzy注170713：
 app版本号
 */
@property (nonatomic, strong) NSString *appVersionString;

@end

@implementation YTKCacheMetadata

/* lzy注170713：
 NSSecureCoding 协议方法：是否支持 安全编码。
 NSSecureCoding继承NSCoding
 */
+ (BOOL)supportsSecureCoding {
    return YES;
}
/* lzy注170713：
 NSCoding协议 方法。
 */
- (void)encodeWithCoder:(NSCoder *)aCoder {
    [aCoder encodeObject:@(self.version) forKey:NSStringFromSelector(@selector(version))];
    [aCoder encodeObject:self.sensitiveDataString forKey:NSStringFromSelector(@selector(sensitiveDataString))];
    [aCoder encodeObject:@(self.stringEncoding) forKey:NSStringFromSelector(@selector(stringEncoding))];
    [aCoder encodeObject:self.creationDate forKey:NSStringFromSelector(@selector(creationDate))];
    [aCoder encodeObject:self.appVersionString forKey:NSStringFromSelector(@selector(appVersionString))];
}
/* lzy注170713：
 NSCoding协议 方法。
 */
- (nullable instancetype)initWithCoder:(NSCoder *)aDecoder {
    self = [self init];
    if (!self) {
        return nil;
    }

    self.version = [[aDecoder decodeObjectOfClass:[NSNumber class] forKey:NSStringFromSelector(@selector(version))] integerValue];
    self.sensitiveDataString = [aDecoder decodeObjectOfClass:[NSString class] forKey:NSStringFromSelector(@selector(sensitiveDataString))];
    self.stringEncoding = [[aDecoder decodeObjectOfClass:[NSNumber class] forKey:NSStringFromSelector(@selector(stringEncoding))] integerValue];
    self.creationDate = [aDecoder decodeObjectOfClass:[NSDate class] forKey:NSStringFromSelector(@selector(creationDate))];
    self.appVersionString = [aDecoder decodeObjectOfClass:[NSString class] forKey:NSStringFromSelector(@selector(appVersionString))];

    return self;
}

@end

@interface YTKRequest()
/* lzy注170713：
 缓存的二进制数据格式
 */
@property (nonatomic, strong) NSData *cacheData;
/* lzy注170713：
 缓存数据的字符串格式
 */
@property (nonatomic, strong) NSString *cacheString;
/* lzy注170713：
 缓存数据的JSON格式
 */
@property (nonatomic, strong) id cacheJSON;
/* lzy注170713：
 缓存数据的XML格式
 */
@property (nonatomic, strong) NSXMLParser *cacheXML;
/* lzy注170713：
 缓存数据的 元数据信息类
 */
@property (nonatomic, strong) YTKCacheMetadata *cacheMetadata;
/* lzy注170713：
 在头文件中，isDataFromCache方法暴露。
 数据是否来自于缓存。
 */
@property (nonatomic, assign) BOOL dataFromCache;

@end

@implementation YTKRequest

/* lzy注170713：
 开启一个请求。
 1、请求是否使用缓存作为响应数据，默认为NO，next；
 当为YES时，将本类缓存数据相关变量置空，调用startWithoutCache方法。
 2、是否断点续传（不可缓存下载请求），否，next，
 yes，调用startWithoutCache方法。
 3、加载缓存数据。加载成功，next；
 加载失败，调用startWithoutCache方法。
 4、数据是否来自于缓存 的标识设置为YES。
 5、在主线程做如下事情：
     5.1 调用请求完成处理器方法
     5.2 调用请求完成过滤器方法
     5.3 请求成功的回调
     5.4 置空本类的成功失败回调（这也是YTK基础教程中这段话的依据：『注意：你可以直接在 block 回调中使用 self，不用担心循环引用。因为 YTKRequest 会在执行完 block 回调之后，将相应的 block 设置成 nil。从而打破循环引用。』）
 */
- (void)start {
    if (self.ignoreCache) {
        [self startWithoutCache];
        return;
    }

    // Do not cache download request.
    if (self.resumableDownloadPath) {
        [self startWithoutCache];
        return;
    }

    if (![self loadCacheWithError:nil]) {
        [self startWithoutCache];
        return;
    }

    _dataFromCache = YES;

    dispatch_async(dispatch_get_main_queue(), ^{
        [self requestCompletePreprocessor];
        [self requestCompleteFilter];
        YTKRequest *strongSelf = self;
        [strongSelf.delegate requestFinished:strongSelf];
        if (strongSelf.successCompletionBlock) {
            strongSelf.successCompletionBlock(strongSelf);
        }
        [strongSelf clearCompletionBlock];
    });
}

/* lzy注170713：
 不考虑缓存，直接开始请求
 1、将本类缓存相关的变量置空
 2、调用父类的start方法
 */
- (void)startWithoutCache {
    
    [self clearCacheVariables];
    
    [super start];
}

#pragma mark - Network Request Delegate
/* lzy注170713：
 请求完成处理方法
 1、调用父类方法
 2、缓存数据写入磁盘
 writeCacheAsynchronously默认值YES，缓存写入存储的操作是否是异步的。
 YES，调用 缓存写入队列，调用saveResponseDataToCacheFile方法
 NO，直接在当前队列调用saveResponseDataToCacheFile方法
 */
- (void)requestCompletePreprocessor {
    [super requestCompletePreprocessor];

    if (self.writeCacheAsynchronously) {
        dispatch_async(ytkrequest_cache_writing_queue(), ^{
            [self saveResponseDataToCacheFile:[super responseData]];
        });
    } else {
        [self saveResponseDataToCacheFile:[super responseData]];
    }
}

#pragma mark - Subclass Override

- (NSInteger)cacheTimeInSeconds {
    return -1;
}

- (long long)cacheVersion {
    return 0;
}

- (id)cacheSensitiveData {
    return nil;
}

- (BOOL)writeCacheAsynchronously {
    return YES;
}

#pragma mark -

- (BOOL)isDataFromCache {
    return _dataFromCache;
}

- (NSData *)responseData {
    if (_cacheData) {
        return _cacheData;
    }
    return [super responseData];
}

- (NSString *)responseString {
    if (_cacheString) {
        return _cacheString;
    }
    return [super responseString];
}

- (id)responseJSONObject {
    if (_cacheJSON) {
        return _cacheJSON;
    }
    return [super responseJSONObject];
}

- (id)responseObject {
    if (_cacheJSON) {
        return _cacheJSON;
    }
    if (_cacheXML) {
        return _cacheXML;
    }
    if (_cacheData) {
        return _cacheData;
    }
    return [super responseObject];
}

#pragma mark -
/* lzy注170713：
  手动从存储中加载缓存。
 1、判断cacheTimeInSeconds值是否为正数，否则生成一个错误给外部传入的error赋值，并返回加载缓存失败的返回值。
 2、尝试加载 缓存元数据信息
 3、通过 元数据信息，判断缓存数据是否过期
 4、尝试加载缓存数据
 */
- (BOOL)loadCacheWithError:(NSError * _Nullable __autoreleasing *)error {
    // Make sure cache time in valid.
    if ([self cacheTimeInSeconds] < 0) {
        if (error) {
            *error = [NSError errorWithDomain:YTKRequestCacheErrorDomain code:YTKRequestCacheErrorInvalidCacheTime userInfo:@{ NSLocalizedDescriptionKey:@"Invalid cache time"}];
        }
        return NO;
    }

    // Try load metadata.
    if (![self loadCacheMetadata]) {
        if (error) {
            *error = [NSError errorWithDomain:YTKRequestCacheErrorDomain code:YTKRequestCacheErrorInvalidMetadata userInfo:@{ NSLocalizedDescriptionKey:@"Invalid metadata. Cache may not exist"}];
        }
        return NO;
    }

    // Check if cache is still valid.
    if (![self validateCacheWithError:error]) {
        return NO;
    }

    // Try load cache.
    if (![self loadCacheData]) {
        if (error) {
            *error = [NSError errorWithDomain:YTKRequestCacheErrorDomain code:YTKRequestCacheErrorInvalidCacheData userInfo:@{ NSLocalizedDescriptionKey:@"Invalid cache data"}];
        }
        return NO;
    }

    return YES;
}

/* lzy注170713：
 通过 元数据信息，判断缓存数据是否过期。
 1、缓存创建时间距离目前的时间差。
 时间差未负数或者超过了开发者设置的cacheTimeInSeconds，生成错误给传入的error赋值，并返回 缓存无效
 2、当前请求的 缓存版本标识，与元数据信息中的 缓存版本标识 不一致。
 生成错误给传入的error赋值，并返回 缓存无效
 3、当前请求的 敏感信息字符串，与元数据信息中的 敏感信息字符串 不一致。
 生成错误给传入的error赋值，并返回 缓存无效
 4、当前请求的 app版本号，与元数据信息中的 app版本号 不一致。
 生成错误给传入的error赋值，并返回 缓存无效
 */
- (BOOL)validateCacheWithError:(NSError * _Nullable __autoreleasing *)error {
    // Date
    NSDate *creationDate = self.cacheMetadata.creationDate;
    NSTimeInterval duration = -[creationDate timeIntervalSinceNow];
    if (duration < 0 || duration > [self cacheTimeInSeconds]) {
        if (error) {
            *error = [NSError errorWithDomain:YTKRequestCacheErrorDomain code:YTKRequestCacheErrorExpired userInfo:@{ NSLocalizedDescriptionKey:@"Cache expired"}];
        }
        return NO;
    }
    // Version
    long long cacheVersionFileContent = self.cacheMetadata.version;
    if (cacheVersionFileContent != [self cacheVersion]) {
        if (error) {
            *error = [NSError errorWithDomain:YTKRequestCacheErrorDomain code:YTKRequestCacheErrorVersionMismatch userInfo:@{ NSLocalizedDescriptionKey:@"Cache version mismatch"}];
        }
        return NO;
    }
    // Sensitive data
    NSString *sensitiveDataString = self.cacheMetadata.sensitiveDataString;
    NSString *currentSensitiveDataString = ((NSObject *)[self cacheSensitiveData]).description;
    if (sensitiveDataString || currentSensitiveDataString) {
        // If one of the strings is nil, short-circuit evaluation will trigger
        /* lzy注170713：
         两个字符串只要有一个为空，表达式值就出来了
         */
        if (sensitiveDataString.length != currentSensitiveDataString.length || ![sensitiveDataString isEqualToString:currentSensitiveDataString]) {
            if (error) {
                *error = [NSError errorWithDomain:YTKRequestCacheErrorDomain code:YTKRequestCacheErrorSensitiveDataMismatch userInfo:@{ NSLocalizedDescriptionKey:@"Cache sensitive data mismatch"}];
            }
            return NO;
        }
    }
    // App version
    NSString *appVersionString = self.cacheMetadata.appVersionString;
    NSString *currentAppVersionString = [YTKNetworkUtils appVersionString];
    if (appVersionString || currentAppVersionString) {
        if (appVersionString.length != currentAppVersionString.length || ![appVersionString isEqualToString:currentAppVersionString]) {
            if (error) {
                *error = [NSError errorWithDomain:YTKRequestCacheErrorDomain code:YTKRequestCacheErrorAppVersionMismatch userInfo:@{ NSLocalizedDescriptionKey:@"App version mismatch"}];
            }
            return NO;
        }
    }
    return YES;
}
/* lzy注170713：
 加载缓存的元数据信息。
 1、获取元素据存储的路径
 2、判断元数据是否存在，在存在时尝试读取加载到内存中，复制给本类属性cacheMetadata。
 并catch住，如果加载到内存中失败时的异常，以框架Log的形式打印
 */
- (BOOL)loadCacheMetadata {
    NSString *path = [self cacheMetadataFilePath];
    NSFileManager * fileManager = [NSFileManager defaultManager];
    if ([fileManager fileExistsAtPath:path isDirectory:nil]) {
        @try {
            _cacheMetadata = [NSKeyedUnarchiver unarchiveObjectWithFile:path];
            return YES;
        } @catch (NSException *exception) {
            YTKLog(@"Load cache metadata failed, reason = %@", exception.reason);
            return NO;
        }
    }
    return NO;
}

/* lzy注170713：
 加载缓存数据。
 1、缓存数据路径
 2、缓存数据存在则读取其二进制数据并赋值给对应属性。
 根据读取到二进制数据，已经缓存数据规定的字符串编码格式（保存在元数据信息类中），生成字符串并赋值给对应属性。
 3、根据 请求 的响应数据类型，决定是直接返回二进制数据、还是解析成JSON或XML对象后，对应返回。
 */
- (BOOL)loadCacheData {
    NSString *path = [self cacheFilePath];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSError *error = nil;

    if ([fileManager fileExistsAtPath:path isDirectory:nil]) {
        NSData *data = [NSData dataWithContentsOfFile:path];
        _cacheData = data;
        _cacheString = [[NSString alloc] initWithData:_cacheData encoding:self.cacheMetadata.stringEncoding];
        switch (self.responseSerializerType) {
            case YTKResponseSerializerTypeHTTP:
                // Do nothing.
                return YES;
            case YTKResponseSerializerTypeJSON:
                _cacheJSON = [NSJSONSerialization JSONObjectWithData:_cacheData options:(NSJSONReadingOptions)0 error:&error];
                return error == nil;
            case YTKResponseSerializerTypeXMLParser:
                _cacheXML = [[NSXMLParser alloc] initWithData:_cacheData];
                return YES;
        }
    }
    return NO;
}

/* lzy注170713：
 保存响应数据至缓存文件
 1、判断是否执行下一步，cacheTimeInSeconds是正值 且 内存数据本身不是来自缓存，next
 2、二进制数据不为空，next
 3、异常捕获代码块，捕获以下两步可能导致的异常，使用log打印
     4、缓存写入 缓存路径
     5、生成 缓存元数据信息对象，并给相关属性赋值，并将缓存元数据信息 写入 元数据信息缓存路径
 
 */
- (void)saveResponseDataToCacheFile:(NSData *)data {
    if ([self cacheTimeInSeconds] > 0 && ![self isDataFromCache]) {
        if (data != nil) {
            @try {
                // New data will always overwrite old data.
                [data writeToFile:[self cacheFilePath] atomically:YES];

                YTKCacheMetadata *metadata = [[YTKCacheMetadata alloc] init];
                metadata.version = [self cacheVersion];
                metadata.sensitiveDataString = ((NSObject *)[self cacheSensitiveData]).description;
                metadata.stringEncoding = [YTKNetworkUtils stringEncodingWithRequest:self];
                metadata.creationDate = [NSDate date];
                metadata.appVersionString = [YTKNetworkUtils appVersionString];
                [NSKeyedArchiver archiveRootObject:metadata toFile:[self cacheMetadataFilePath]];
            } @catch (NSException *exception) {
                YTKLog(@"Save cache failed, reason = %@", exception.reason);
            }
        }
    }
}
/* lzy注170713：
 将缓存相关的变量置空
 */
- (void)clearCacheVariables {
    _cacheData = nil;
    _cacheXML = nil;
    _cacheJSON = nil;
    _cacheString = nil;
    _cacheMetadata = nil;
    _dataFromCache = NO;
}

#pragma mark -
/* lzy注170713：
 按需创建path文件夹
 1、是否存在，不存在，创建 缓存文件夹
 2、文件存在但不是文件夹，创建 缓存文件夹
 */
- (void)createDirectoryIfNeeded:(NSString *)path {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    BOOL isDir;
    if (![fileManager fileExistsAtPath:path isDirectory:&isDir]) {
        [self createBaseDirectoryAtPath:path];
    } else {
        if (!isDir) {
            NSError *error = nil;
            [fileManager removeItemAtPath:path error:&error];
            [self createBaseDirectoryAtPath:path];
        }
    }
}
/* lzy注170713：
 创建path文件夹操作，若有中间文件夹也一并创建。
 创建出错，打印log
 创建成功，给该路径添加 不需备份 属性
 */
- (void)createBaseDirectoryAtPath:(NSString *)path {
    NSError *error = nil;
    [[NSFileManager defaultManager] createDirectoryAtPath:path withIntermediateDirectories:YES
                                               attributes:nil error:&error];
    if (error) {
        YTKLog(@"create cache directory failed, error = %@", error);
    } else {
        [YTKNetworkUtils addDoNotBackupAttribute:path];
    }
}
/* lzy注170713：
 任何缓存的主路径。
 1、sandbox/Library/LazyRequestCache
 2、根据YTKNetworkConfig中配置的cacheDirPathFilters生成一个路径
 3、判断文件夹是否存在，不存在即创建
 */
- (NSString *)cacheBasePath {
    NSString *pathOfLibrary = [NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    NSString *path = [pathOfLibrary stringByAppendingPathComponent:@"LazyRequestCache"];

    // Filter cache base path
    NSArray<id<YTKCacheDirPathFilterProtocol>> *filters = [[YTKNetworkConfig sharedConfig] cacheDirPathFilters];
    if (filters.count > 0) {
        for (id<YTKCacheDirPathFilterProtocol> f in filters) {
            path = [f filterCacheDirPath:path withRequest:self];
        }
    }

    [self createDirectoryIfNeeded:path];
    return path;
}
/* lzy注170713：
 获得缓存某数据时的名称。
 1、一些请求的基本信息拼接。requestMethod、baseUrl、requestUrl、argument
 2、拼接的字符串md5一下
 */
- (NSString *)cacheFileName {
    NSString *requestUrl = [self requestUrl];
    NSString *baseUrl = [YTKNetworkConfig sharedConfig].baseUrl;
    id argument = [self cacheFileNameFilterForRequestArgument:[self requestArgument]];
    NSString *requestInfo = [NSString stringWithFormat:@"Method:%ld Host:%@ Url:%@ Argument:%@",
                             (long)[self requestMethod], baseUrl, requestUrl, argument];
    NSString *cacheFileName = [YTKNetworkUtils md5StringFromString:requestInfo];
    return cacheFileName;
}
/* lzy注170713：
 缓存数据路径。
 1、获得缓存某数据时的名称。
 2、获取缓存的主路径
 3、两者拼接
 */
- (NSString *)cacheFilePath {
    NSString *cacheFileName = [self cacheFileName];
    NSString *path = [self cacheBasePath];
    path = [path stringByAppendingPathComponent:cacheFileName];
    return path;
}
/* lzy注170713：
 缓存元数据的路径。
 1、缓存数据的名称拼接`.metadata`
 2、获取缓存的主路径
 3、两者拼接
 */
- (NSString *)cacheMetadataFilePath {
    NSString *cacheMetadataFileName = [NSString stringWithFormat:@"%@.metadata", [self cacheFileName]];
    NSString *path = [self cacheBasePath];
    path = [path stringByAppendingPathComponent:cacheMetadataFileName];
    return path;
}

@end
