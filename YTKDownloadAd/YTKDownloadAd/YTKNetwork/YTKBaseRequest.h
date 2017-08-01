//
//  YTKBaseRequest.h
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

#import <Foundation/Foundation.h>
/* lzy注170713：
 1. 这个类头文件从NS_ASSUME_NONNULL_BEGIN开始，表示到NS_ASSUME_NONNULL_END之间的所有简单指针对象都被假定为nonnull。
 */
NS_ASSUME_NONNULL_BEGIN

/* lzy注170714：
 请求有效性 错误域名
 */
FOUNDATION_EXPORT NSString *const YTKRequestValidationErrorDomain;

/* lzy注170714：
 请求有效性 - 无效状态枚举
 */
NS_ENUM(NSInteger) {
    YTKRequestValidationErrorInvalidStatusCode = -8,
    YTKRequestValidationErrorInvalidJSONFormat = -9,
};

///  HTTP Request method.
    /* lzy注170714：
     YTK封装的请求方法枚举
     */
typedef NS_ENUM(NSInteger, YTKRequestMethod) {
    YTKRequestMethodGET = 0,
    YTKRequestMethodPOST,
    YTKRequestMethodHEAD,
    YTKRequestMethodPUT,
    YTKRequestMethodDELETE,
    YTKRequestMethodPATCH,
};

///  Request serializer type.
    /* lzy注170714：
     请求序列化类型枚举
     */
typedef NS_ENUM(NSInteger, YTKRequestSerializerType) {
    YTKRequestSerializerTypeHTTP = 0,
    YTKRequestSerializerTypeJSON,
};

///  Response serializer type, which determines response serialization process and
///  the type of `responseObject`.
    /* lzy注170714：
     响应数据 序列化类型，将决定响应数据进行哪种序列化，以及`responseObject`的类型是什么
     */
typedef NS_ENUM(NSInteger, YTKResponseSerializerType) {
    /// NSData type
    YTKResponseSerializerTypeHTTP,
    /// JSON object type
    YTKResponseSerializerTypeJSON,
    /// NSXMLParser type
    YTKResponseSerializerTypeXMLParser,
};

///  Request priority
    /* lzy注170714：
     请求 优先级
     */
typedef NS_ENUM(NSInteger, YTKRequestPriority) {
    YTKRequestPriorityLow = -4L,
    YTKRequestPriorityDefault = 0,
    YTKRequestPriorityHigh = 4,
};

    /* lzy注170714：
      The `AFMultipartFormData` protocol defines the methods supported by the parameter in the block argument of `AFHTTPRequestSerializer -multipartFormRequestWithMethod:URLString:parameters:constructingBodyWithBlock:`.
     */
@protocol AFMultipartFormData;

typedef void (^AFConstructingBlock)(id<AFMultipartFormData> formData);
typedef void (^AFURLSessionTaskProgressBlock)(NSProgress *);

@class YTKBaseRequest;
    /* lzy注170714：
     申明了一个block的别名，这个block是ytk请求完成的回调
     */
typedef void(^YTKRequestCompletionBlock)(__kindof YTKBaseRequest *request);
    

///  The YTKRequestDelegate protocol defines several optional methods you can use
///  to receive network-related messages. All the delegate methods will be called
///  on the main queue.
    /* lzy注170714：
     定义了几个可选方法，可用用于接收网络相关的消息。所有的delegate方法都将在主队列被调用。
     */
@protocol YTKRequestDelegate <NSObject>

@optional
///  Tell the delegate that the request has finished successfully.
///
///  @param request The corresponding request.
- (void)requestFinished:(__kindof YTKBaseRequest *)request;

///  Tell the delegate that the request has failed.
///
///  @param request The corresponding request.
- (void)requestFailed:(__kindof YTKBaseRequest *)request;

@end

///  The YTKRequestAccessory protocol defines several optional methods that can be
///  used to track the status of a request. Objects that conforms this protocol
///  ("accessories") can perform additional configurations accordingly. All the
///  accessory methods will be called on the main queue.
    /* lzy注170714：
     定义了几个可选方法，可用于跟踪一个请求的状态。遵守YTKRequestAccessory这个协议的对象，可以执行相应的额外配置，所有的配件方法将在主队列被调用。
     */
@protocol YTKRequestAccessory <NSObject>

@optional

///  Inform the accessory that the request is about to start.
///
///  @param request The corresponding request.
- (void)requestWillStart:(id)request;

///  Inform the accessory that the request is about to stop. This method is called
///  before executing `requestFinished` and `successCompletionBlock`.
///
///  @param request The corresponding request.
- (void)requestWillStop:(id)request;

///  Inform the accessory that the request has already stoped. This method is called
///  after executing `requestFinished` and `successCompletionBlock`.
///
///  @param request The corresponding request.
- (void)requestDidStop:(id)request;

@end

///  YTKBaseRequest is the abstract class of network request. It provides many options
///  for constructing request. It's the base class of `YTKRequest`.
    /* lzy注170714：
     网络请求的抽象类。
     提供了构建一个网络请求的多种选择。它是`YTKRequest`类的父类。
     */
@interface YTKBaseRequest : NSObject

#pragma mark - Request and Response Information
///=============================================================================
/// @name Request and Response Information
///=============================================================================

///  The underlying NSURLSessionTask.
///
///  @warning This value is actually nil and should not be accessed before the request starts.
/* lzy注170714：
 基础的NSURLSessionTask。这个值在请求开始之前为空，应当在请求开始后来访问这个属性。
 */
@property (nonatomic, strong, readonly) NSURLSessionTask *requestTask;


/* lzy注170714：
 一下两个属性，是NSURLSessionTask的基本属性。
 currentRequest：The URL request object currently being handled by the task.
 This value is typically the same as the initial request (originalRequest) except when the server has responded to the initial request with a redirect to a different URL.
 
 originalRequest：The original request object passed when the task was created.
 This value is typically the same as the currently active request (currentRequest) except when the server has responded to the initial request with a redirect to a different URL.
 currentRequest、originalRequest意思顾名思义。两者一般是一致的，除非，服务器将原始请求重定向到了另一个URL。
 */
///  Shortcut for `requestTask.currentRequest`.
/* lzy注170714：
 `requestTask.currentRequest`的快捷访问
 */
@property (nonatomic, strong, readonly) NSURLRequest *currentRequest;

///  Shortcut for `requestTask.originalRequest`.
/* lzy注170714：
 `requestTask.originalRequest`的快捷访问。
 */
@property (nonatomic, strong, readonly) NSURLRequest *originalRequest;



///  Shortcut for `requestTask.response`.
@property (nonatomic, strong, readonly) NSHTTPURLResponse *response;

///  The response status code.
@property (nonatomic, readonly) NSInteger responseStatusCode;

///  The response header fields.
@property (nonatomic, strong, readonly, nullable) NSDictionary *responseHeaders;

///  The raw data representation of response. Note this value can be nil if request failed.
@property (nonatomic, strong, readonly, nullable) NSData *responseData;

///  The string representation of response. Note this value can be nil if request failed.
@property (nonatomic, strong, readonly, nullable) NSString *responseString;

///  This serialized response object. The actual type of this object is determined by
///  `YTKResponseSerializerType`. Note this value can be nil if request failed.
///
///  @discussion If `resumableDownloadPath` and DownloadTask is using, this value will
///              be the path to which file is successfully saved (NSURL), or nil if request failed.
/* lzy注170714：
 序列化了响应数据之后得到的对象。它的类型由`YTKResponseSerializerType`决定。
 这个值在请求失败时为nil。
 讨论：如果`resumableDownloadPath` and DownloadTask被使用，这个值将是文件被成功保存所在的路径。
 */
@property (nonatomic, strong, readonly, nullable) id responseObject;

///  If you use `YTKResponseSerializerTypeJSON`, this is a convenience (and sematic) getter
///  for the response object. Otherwise this value is nil.
@property (nonatomic, strong, readonly, nullable) id responseJSONObject;

///  This error can be either serialization error or network error. If nothing wrong happens
///  this value will be nil.
/* lzy注170714：
 这个错误可能是序列化或者网络错误的信息。
 */
@property (nonatomic, strong, readonly, nullable) NSError *error;

///  Return cancelled state of request task.
/* lzy注170714：
 请求task是否取消成功。
 */
@property (nonatomic, readonly, getter=isCancelled) BOOL cancelled;

///  Executing state of request task.
/* lzy注170714：
 请求task是否执行成功。
 */
@property (nonatomic, readonly, getter=isExecuting) BOOL executing;


#pragma mark - Request Configuration
///=============================================================================
/// @name Request Configuration
///=============================================================================

///  Tag can be used to identify request. Default value is 0.
/* lzy注170714：
 一个可以用于标识请求的变量。默认为0。
 */
@property (nonatomic) NSInteger tag;

///  The userInfo can be used to store additional info about the request. Default is nil.
/* lzy注170714：
 默认为nil。可用于存储请求的相关信息。
 */
@property (nonatomic, strong, nullable) NSDictionary *userInfo;

///  The delegate object of the request. If you choose block style callback you can ignore this.
///  Default is nil.
/* lzy注170714：
 一个请求的delegate对象。如果你使用blcck类型的回调，可以忽略这个属性。默认值是nil。
 */
@property (nonatomic, weak, nullable) id<YTKRequestDelegate> delegate;

///  The success callback. Note if this value is not nil and `requestFinished` delegate method is
///  also implemented, both will be executed but delegate method is first called. This block
///  will be called on the main queue.
/* lzy注170714：
 请求成功的回调。
 若使用blcok类型回调，同时遵守了YTKRequestDelegate协议，实现了`requestFinished`delegate方法，那么两种回调都将执行，且delegate方法将先被调用。
 这个block将在主线程回调。
 */
@property (nonatomic, copy, nullable) YTKRequestCompletionBlock successCompletionBlock;

///  The failure callback. Note if this value is not nil and `requestFailed` delegate method is
///  also implemented, both will be executed but delegate method is first called. This block
///  will be called on the main queue.
/* lzy注170714：
 请求失败的回调。
 若使用blcok类型回调，同时遵守了YTKRequestDelegate协议，实现了`requestFailed`delegate方法，那么两种回调都将执行，且delegate方法将先被调用。
 这个block将在主线程回调。
 */
@property (nonatomic, copy, nullable) YTKRequestCompletionBlock failureCompletionBlock;

///  This can be used to add several accossories object. Note if you use `addAccessory` to add acceesory
///  this array will be automatically created. Default is nil.
/* lzy注170714：
 可用于添加多个配件对象。如果使用`addAccessory`方法添加配件，那么这个数组将被自动创建。默认值为nil。
 */
@property (nonatomic, strong, nullable) NSMutableArray<id<YTKRequestAccessory>> *requestAccessories;

///  This can be use to construct HTTP body when needed in POST request. Default is nil.
/* lzy注170714：
 在POST请求发送数据时，可用于构建HTTP body。默认值为nil。
 */
@property (nonatomic, copy, nullable) AFConstructingBlock constructingBodyBlock;

///  This value is used to perform resumable download request. Default is nil.
///
///  @discussion NSURLSessionDownloadTask is used when this value is not nil.
///              The exist file at the path will be removed before the request starts. If request succeed, file will
///              be saved to this path automatically, otherwise the response will be saved to `responseData`
///              and `responseString`. For this to work, server must support `Range` and response with
///              proper `Last-Modified` and/or `Etag`. See `NSURLSessionDownloadTask` for more detail.
/* lzy注170714：
 这个值被用于执行可恢复下载请求市。默认是nil。
 讨论：当这个值不是nil时，NSURLSessionDownloadTask是被启用的。如果是下载任务已经存在于这个路径的文件，将在请求开始之前被删除。当这个请求成功之后，文件将被自动保存到这个路径，否则，响应数据将被保存在`responseData`和`responseString`中。因此，服务器端需要支持`Range`，而且，返回合适的`Last-Modified` 和/或者 `Etag`。具体可以参看`NSURLSessionDownloadTask`
 */
@property (nonatomic, strong, nullable) NSString *resumableDownloadPath;

///  You can use this block to track the download progress. See also `resumableDownloadPath`.
/* lzy注170714：
 用于跟踪下载进度
 */
@property (nonatomic, copy, nullable) AFURLSessionTaskProgressBlock resumableDownloadProgressBlock;

///  The priority of the request. Effective only on iOS 8+. Default is `YTKRequestPriorityDefault`.
/* lzy注170714：
 请求的优先级。iOS8+。默认为`YTKRequestPriorityDefault`
 */
@property (nonatomic) YTKRequestPriority requestPriority;

///  Set completion callbacks
- (void)setCompletionBlockWithSuccess:(nullable YTKRequestCompletionBlock)success
                              failure:(nullable YTKRequestCompletionBlock)failure;

///  Nil out both success and failure callback blocks.
- (void)clearCompletionBlock;

///  Convenience method to add request accessory. See also `requestAccessories`.
- (void)addAccessory:(id<YTKRequestAccessory>)accessory;


#pragma mark - Request Action
///=============================================================================
/// @name Request Action
///=============================================================================

///  Append self to request queue and start the request.
- (void)start;// -> 调用 YTKNetworkAgent 的 addRequest: 方法

///  Remove self from request queue and cancel the request.
- (void)stop;// -> 调用 YTKNetworkAgent 的 cancelRequest: 方法

///  Convenience method to start the request with block callbacks.
/* lzy注170714：
 快捷得开启一个请求并以block作为回调的方法
 */
- (void)startWithCompletionBlockWithSuccess:(nullable YTKRequestCompletionBlock)success
                                    failure:(nullable YTKRequestCompletionBlock)failure;


#pragma mark - Subclass Override
///=============================================================================
/// @name Subclass Override
///=============================================================================
/* lzy注170713：
 定义了一系列子类需要覆盖的方法，并且本类的.m文件中将这些方法都写出并带有返回值，避免子类不覆盖这些方法，内部又调用到这些方法时造成找不到selector的crash
 */
///  Called on background thread after request succeded but before switching to main thread. Note if
///  cache is loaded, this method WILL be called on the main thread, just like `requestCompleteFilter`.
/* lzy注170714：
 在请求成功后在切换到主线程之前，在后台线程调用本方法。
 如果是请求是加载缓存，这个方法将在主线程调用。
 */
- (void)requestCompletePreprocessor;

///  Called on the main thread after request succeeded.
/* lzy注170714：
 在请求成功后在主线程调用。
 */
- (void)requestCompleteFilter;

///  Called on background thread after request succeded but before switching to main thread. See also
///  `requestCompletePreprocessor`.
/* lzy注170714：
 在请求成功后在切换到主线程之前，在后台线程调用本方法。参看`requestCompletePreprocessor`.
 */
- (void)requestFailedPreprocessor;

///  Called on the main thread when request failed.
/* lzy注170714：
 在请求失败后在主线程调用。
 */
- (void)requestFailedFilter;

///  The baseURL of request. This should only contain the host part of URL, e.g., http://www.example.com.
///  See also `requestUrl`
/* lzy注170714：
 请求的基本URL。只应该包括URL的主机地址部分。参看`requestUrl`。
 */
- (NSString *)baseUrl;

///  The URL path of request. This should only contain the path part of URL, e.g., /v1/user. See alse `baseUrl`.
///
///  @discussion This will be concated with `baseUrl` using [NSURL URLWithString:relativeToURL].
///              Because of this, it is recommended that the usage should stick to rules stated above.
///              Otherwise the result URL may not be correctly formed. See also `URLString:relativeToURL`
///              for more information.
///
///              Additionaly, if `requestUrl` itself is a valid URL, it will be used as the result URL and
///              `baseUrl` will be ignored.
/* lzy注170714：
 请求的URL地址。
 只应该包括URL的文件夹路径部分，参看`baseUrl`。
 讨论：`requestUrl`将和`baseUrl`一起，使用[NSURL URLWithString:relativeToURL]协调。这两个参数务必遵守这个约定，否则协调出来的地址可能不正确。另外，若`requestUrl`本身是一个有效的URL，那么它将被当做结果URL，而`baseUrl`将会被忽略。
 */
- (NSString *)requestUrl;

///  Optional CDN URL for request.
- (NSString *)cdnUrl;

///  Requset timeout interval. Default is 60s.
///
///  @discussion When using `resumableDownloadPath`(NSURLSessionDownloadTask), the session seems to completely ignore
///              `timeoutInterval` property of `NSURLRequest`. One effective way to set timeout would be using
///              `timeoutIntervalForResource` of `NSURLSessionConfiguration`.
/* lzy注170714：
 请求响应超时间隔。默认是60s。
 讨论：当NSURLSessionDownloadTask存在时，即为下载任务时，session似乎不会校验`NSURLRequest`的`timeoutInterval`属性，此时一个有效的设置响应超时的方法是，使用`NSURLSessionConfiguration`的`timeoutIntervalForResource`属性。
 */
- (NSTimeInterval)requestTimeoutInterval;

///  Additional request argument.
- (nullable id)requestArgument;

///  Override this method to filter requests with certain arguments when caching.
/* lzy注170714：
 重写这个方法，可以在缓存时，过滤请求的特定参数
 */
- (id)cacheFileNameFilterForRequestArgument:(id)argument;

///  HTTP request method.
- (YTKRequestMethod)requestMethod;

///  Request serializer type.
- (YTKRequestSerializerType)requestSerializerType;

///  Response serializer type. See also `responseObject`.
- (YTKResponseSerializerType)responseSerializerType;

///  Username and password used for HTTP authorization. Should be formed as @[@"Username", @"Password"].
- (nullable NSArray<NSString *> *)requestAuthorizationHeaderFieldArray;

///  Additional HTTP request header field.
- (nullable NSDictionary<NSString *, NSString *> *)requestHeaderFieldValueDictionary;

///  Use this to build custom request. If this method return non-nil value, `requestUrl`, `requestTimeoutInterval`,
///  `requestArgument`, `allowsCellularAccess`, `requestMethod` and `requestSerializerType` will all be ignored.
/* lzy注170714：
 用于构建自定义的请求。如果这个方法返回非空值，那么原本请求配置的、ytk默认的参数`requestUrl`, `requestTimeoutInterval`,
 ///  `requestArgument`, `allowsCellularAccess`, `requestMethod` and `requestSerializerType`都将被忽略而。
 */
- (nullable NSURLRequest *)buildCustomUrlRequest;

///  Should use CDN when sending request.
- (BOOL)useCDN;

///  Whether the request is allowed to use the cellular radio (if present). Default is YES.
- (BOOL)allowsCellularAccess;

///  The validator will be used to test if `responseJSONObject` is correctly formed.
/* lzy注170714：
 这个验证器用于检查`responseJSONObject`是否是正确的格式。
 */
- (nullable id)jsonValidator;

///  This validator will be used to test if `responseStatusCode` is valid.
/* lzy注170714：
 这个验证器将用于检查`responseStatusCode`是否有效。
 */
- (BOOL)statusCodeValidator;

@end

NS_ASSUME_NONNULL_END
