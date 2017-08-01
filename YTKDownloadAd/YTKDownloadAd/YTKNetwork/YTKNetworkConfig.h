//
//  YTKNetworkConfig.h
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
/* lzy注170713：
 使用了YTKNetwork发起请求的所有请求的统一配置中心，这里可以配置如统一的参数过滤、baseurl、cdnurl等。
 */
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class YTKBaseRequest;
@class AFSecurityPolicy;

///  YTKUrlFilterProtocol can be used to append common parameters to requests before sending them.
/* lzy注170713：
 用于在请求发送之前，拼接 需要统一添加的请求参数
 */
@protocol YTKUrlFilterProtocol <NSObject>
///  Preprocess request URL before actually sending them.
///
///  @param originUrl request's origin URL, which is returned by `requestUrl`
///  @param request   request itself
///
///  @return A new url which will be used as a new `requestUrl`
/* lzy注170713：
 YTKUrlFilterProtocol方法，在确定请求发送之前，预处理请求URL
 @param originUrl 请求的原始 URL, 由`requestUrl`决定
 @param request   request 自身
 */
- (NSString *)filterUrl:(NSString *)originUrl withRequest:(YTKBaseRequest *)request;
@end

///  YTKCacheDirPathFilterProtocol can be used to append common path components when caching response results
/* lzy注170713：
 用于缓存响应结果时，拼接 统一的某段路径
 */
@protocol YTKCacheDirPathFilterProtocol <NSObject>
///  Preprocess cache path before actually saving them.
///
///  @param originPath original base cache path, which is generated in `YTKRequest` class.
///  @param request    request itself
///
///  @return A new path which will be used as base path when caching.
/* lzy注170713：
 YTKCacheDirPathFilterProtocol方法。
 在缓存文件之前，预处理缓存路径
 */
- (NSString *)filterCacheDirPath:(NSString *)originPath withRequest:(YTKBaseRequest *)request;
@end

///  YTKNetworkConfig stored global network-related configurations, which will be used in `YTKNetworkAgent`
///  to form and filter requests, as well as caching response.
/* lzy注170714：
 YTKNetworkConfig类，存储着全局的网络相关的配置，这些配置将会被`YTKNetworkAgent`用来设置和过滤请求、缓存响应数据相关的操作。
 
 */
@interface YTKNetworkConfig : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

///  Return a shared config object.
+ (YTKNetworkConfig *)sharedConfig;

///  Request base URL, such as "http://www.yuantiku.com". Default is empty string.
/* lzy注170714：
 请求的基本URL，默认是空字符串
 */
@property (nonatomic, strong) NSString *baseUrl;
///  Request CDN URL. Default is empty string.
/* lzy注170714：
 请求的CDN URL，默认是空字符串
 */
@property (nonatomic, strong) NSString *cdnUrl;
///  URL filters. See also `YTKUrlFilterProtocol`.
/* lzy注170714：
 URL的过滤器。与YTKUrlFilterProtocol有关
 */
@property (nonatomic, strong, readonly) NSArray<id<YTKUrlFilterProtocol>> *urlFilters;
///  Cache path filters. See also `YTKCacheDirPathFilterProtocol`.
/* lzy注170714：
 缓存路径过滤器。和YTKCacheDirPathFilterProtocol有关。
 */
@property (nonatomic, strong, readonly) NSArray<id<YTKCacheDirPathFilterProtocol>> *cacheDirPathFilters;
///  Security policy will be used by AFNetworking. See also `AFSecurityPolicy`.
/* lzy注170714：
 这个安全策略属性将被AFNetworking使用。
 */
@property (nonatomic, strong) AFSecurityPolicy *securityPolicy;
///  Whether to log debug info. Default is NO;
/* lzy注170714：
 调试日志信息是否输出，默认为NO
 */
@property (nonatomic) BOOL debugLogEnabled;
///  SessionConfiguration will be used to initialize AFHTTPSessionManager. Default is nil.
/* lzy注170714：
 用于初始化AFHTTPSessionManager类。默认为nil。
 */
@property (nonatomic, strong) NSURLSessionConfiguration* sessionConfiguration;

///  Add a new URL filter.
/* lzy注170714：
 添加URL过滤器。
 */
- (void)addUrlFilter:(id<YTKUrlFilterProtocol>)filter;
///  Remove all URL filters.
/* lzy注170714：
 移除所有的URL过滤器
 */
- (void)clearUrlFilter;
///  Add a new cache path filter

/**
 添加缓存文件路径过滤器。
 */
- (void)addCacheDirPathFilter:(id<YTKCacheDirPathFilterProtocol>)filter;
///  Clear all cache path filters.
/* lzy注170714：
 移除 缓存文件路径过滤器。
 */
- (void)clearCacheDirPathFilter;

@end

NS_ASSUME_NONNULL_END
