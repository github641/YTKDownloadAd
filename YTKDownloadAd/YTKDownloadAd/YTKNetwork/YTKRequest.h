//
//  YTKRequest.h
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

#import "YTKBaseRequest.h"

NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXPORT NSString *const YTKRequestCacheErrorDomain;
/* lzy注170713：
 使用负数来逐个枚举值，这样代码中可能通过是否<0 ,就可以最快判断是否有错误
 */
NS_ENUM(NSInteger) {
    YTKRequestCacheErrorExpired = -1,
    YTKRequestCacheErrorVersionMismatch = -2,
    YTKRequestCacheErrorSensitiveDataMismatch = -3,
    YTKRequestCacheErrorAppVersionMismatch = -4,
    YTKRequestCacheErrorInvalidCacheTime = -5,
    YTKRequestCacheErrorInvalidMetadata = -6,
    YTKRequestCacheErrorInvalidCacheData = -7,
};
    /* lzy注170713：
     继承自YTKBaseRequest，并额外实现了 缓存的存储和读取
     */
///  YTKRequest is the base class you should inherit to create your own request class.
///  Based on YTKBaseRequest, YTKRequest adds local caching feature. Note download
///  request will not be cached whatsoever, because download request may involve complicated
///  cache control policy controlled by `Cache-Control`, `Last-Modified`, etc.
    /* lzy注170713：
     YTKRequest是创建自己的请求需要继承的父类。
     YTKRequest的父类是YTKBaseRequest。
     YTKRequest相对于父类，添加了本地缓存特性。需要注意，因为下载任务涉及到复杂的缓存控制机制如`Cache-Control`, `Last-Modified`等，所以下载任务是不会做本地缓存的。
     */
@interface YTKRequest : YTKBaseRequest

///  Whether to use cache as response or not.
///  Default is NO, which means caching will take effect with specific arguments.
///  Note that `cacheTimeInSeconds` default is -1. As a result cache data is not actually
///  used as response unless you return a positive value in `cacheTimeInSeconds`.
///
///  Also note that this option does not affect storing the response, which means response will always be saved
///  even `ignoreCache` is YES.
/* lzy注170713：
 是否使用缓存作为响应数据，默认为NO。
 `cacheTimeInSeconds`属性默认是-1。缓存数据实际上不会作为响应数据，除非`cacheTimeInSeconds`的值是正数。
 这个属性并不会影响响应数据的存储动作本身，就算`ignoreCache`被设置为YES，响应数据也是可以被缓存的。
 */
@property (nonatomic) BOOL ignoreCache;

///  Whether data is from local cache.
/* lzy注170713：
 数据是否来自于缓存。
 */
- (BOOL)isDataFromCache;

///  Manually load cache from storage.
///
///  @param error If an error occurred causing cache loading failed, an error object will be passed, otherwise NULL.
///
///  @return Whether cache is successfully loaded.
/* lzy注170713：
 手动从存储中加载缓存。
 */
- (BOOL)loadCacheWithError:(NSError * __autoreleasing *)error;

///  Start request without reading local cache even if it exists. Use this to update local cache.
/* lzy注170713：
 不读取本地缓存，直接开始请求，就算本地有缓存也不读取。用这个方法来更新本地缓存。
 */
- (void)startWithoutCache;

///  Save response data (probably from another request) to this request's cache location
/* lzy注170713：
 保存响应数据（可能来自于另一个请求）到『这个』请求的缓存位置
 */
- (void)saveResponseDataToCacheFile:(NSData *)data;

#pragma mark - Subclass Override

/* lzy注170713：
 子类需要重写的方法
 */
///  The max time duration that cache can stay in disk until it's considered expired.
///  Default is -1, which means response is not actually saved as cache.
/* lzy注170713：
 缓存数据存储在硬盘中的有效期。默认值是-1，意味着响应数据并不会被存储。
 */
- (NSInteger)cacheTimeInSeconds;

///  Version can be used to identify and invalidate local cache. Default is 0.
/* lzy注170713：
 用于标识本地缓存、让本地缓存失效，默认值0
 */
- (long long)cacheVersion;

///  This can be used as additional identifier that tells the cache needs updating.
///
///  @discussion The `description` string of this object will be used as an identifier to verify whether cache
///              is invalid. Using `NSArray` or `NSDictionary` as return value type is recommended. However,
///              If you intend to use your custom class type, make sure that `description` is correctly implemented.
/* lzy注170713：
 可以作为附加的标识，用于通知缓存更新。
 本类的`description`字符串将作为校验缓存是否有效的标识。
 推荐使用`NSArray` or `NSDictionary`作为返回值。如果使用自定义的类，请保证它的`description`方法被正确实现。
 */
- (nullable id)cacheSensitiveData;

///  Whether cache is asynchronously written to storage. Default is YES.
/* lzy注170713：
 默认值YES，缓存写入存储的操作是否是异步的。
 */
- (BOOL)writeCacheAsynchronously;

@end

NS_ASSUME_NONNULL_END
