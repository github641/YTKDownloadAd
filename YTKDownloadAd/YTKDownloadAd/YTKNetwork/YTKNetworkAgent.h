//
//  YTKNetworkAgent.h
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
 网络请求的单例，内部的请求是通过AFHTTPSessionManager来发出的。其中
 
 (void)addRequest:(YTKBaseRequest *)request
 接口会将当前请求启动，并记录下这次请求的信息。
 
 断点继续下载：此类保存了上次下载未完成的文件，再次下载同一个文件时，优先考虑继续上次下载的位置。
 */
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class YTKBaseRequest;

///  YTKNetworkAgent is the underlying class that handles actual request generation,
///  serialization and response handling.
/* lzy注170714：
 本类是实际处理请求生命周期的基本类。
 并对序列化和响应做了处理。
 */
@interface YTKNetworkAgent : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

///  Get the shared agent.
+ (YTKNetworkAgent *)sharedAgent;

///  Add request to session and start it.
/* lzy注170714：
 把请求添加到session中，并启动它
 */
- (void)addRequest:(YTKBaseRequest *)request;

///  Cancel a request that was previously added.
/* lzy注170714：
 取消之前添加的请求
 */
- (void)cancelRequest:(YTKBaseRequest *)request;

///  Cancel all requests that were previously added.
/* lzy注170714：
 取消之前添加的所有请求
 */
- (void)cancelAllRequests;

///  Return the constructed URL of request.
///
///  @param request The request to parse. Should not be nil.
///
///  @return The result URL.
/* lzy注170714：
 返回一个构造好的请求URL。
 需要解析的请求，传入值不能为nil。
 */
- (NSString *)buildRequestUrl:(YTKBaseRequest *)request;

@end

NS_ASSUME_NONNULL_END
