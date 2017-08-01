//
//  YTKBatchRequest.h
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
 实现批量请求，并YTKBatchRequestAgent通过管理（添加和删除）所有请求，当批量请求开始时将遍历YTKBatchRequestAgent保存的所有请求，逐个开始。 遗憾的是，这里的源码并没有对并发的线程做任何管理，如并发数目等限制。
 
 YTKBatchRequest 类：用于方便地发送批量的网络请求，YTKBatchRequest 是一个容器类，它可以放置多个 YTKRequest 子类，并统一处理这多个网络请求的成功和失败。
 
 在如下的示例中，我们发送了 4 个批量的请求，并统一处理这 4 个请求同时成功的回调。
 
 #import "YTKBatchRequest.h"
 #import "GetImageApi.h"
 #import "GetUserInfoApi.h"
 
 - (void)sendBatchRequest {
 GetImageApi *a = [[GetImageApi alloc] initWithImageId:@"1.jpg"];
 GetImageApi *b = [[GetImageApi alloc] initWithImageId:@"2.jpg"];
 GetImageApi *c = [[GetImageApi alloc] initWithImageId:@"3.jpg"];
 GetUserInfoApi *d = [[GetUserInfoApi alloc] initWithUserId:@"123"];
 
 YTKBatchRequest *batchRequest = [[YTKBatchRequest alloc] initWithRequestArray:@[a, b, c, d]];
 
 [batchRequest startWithCompletionBlockWithSuccess:^(YTKBatchRequest *batchRequest) {
 
     NSLog(@"succeed");
 
     NSArray *requests = batchRequest.requestArray;
     GetImageApi *a = (GetImageApi *)requests[0];
     GetImageApi *b = (GetImageApi *)requests[1];
     GetImageApi *c = (GetImageApi *)requests[2];
 
     GetUserInfoApi *user = (GetUserInfoApi *)requests[3];
         // deal with requests result ...
     } failure:^(YTKBatchRequest *batchRequest) {
         NSLog(@"failed");
     }];
 }
 */
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class YTKRequest;
@class YTKBatchRequest;
@protocol YTKRequestAccessory;

///  The YTKBatchRequestDelegate protocol defines several optional methods you can use
///  to receive network-related messages. All the delegate methods will be called
///  on the main queue. Note the delegate methods will be called when all the requests
///  of batch request finishes.
/* lzy注170718：
 YTKBatchRequestDelegate协议 定义了几个可选方法，你可以使用这些可选方法来接收 网络相关的消息。
 所有的delegate方法都将在主线程回调。注意，所有的delegate方法只会在 批量请求的所有请求 都结束之后调用。
 */
@protocol YTKBatchRequestDelegate <NSObject>

@optional
///  Tell the delegate that the batch request has finished successfully/
///
///  @param batchRequest The corresponding batch request.
- (void)batchRequestFinished:(YTKBatchRequest *)batchRequest;

///  Tell the delegate that the batch request has failed.
///
///  @param batchRequest The corresponding batch request.
- (void)batchRequestFailed:(YTKBatchRequest *)batchRequest;

@end

///  YTKBatchRequest can be used to batch several YTKRequest. Note that when used inside YTKBatchRequest, a single
///  YTKRequest will have its own callback and delegate cleared, in favor of the batch request callback.
/* lzy注170718：
 YTKBatchRequest 可以用于批量请求。
 注意，当使用批量请求时，每一个YTKRequest都将有它自己的callback和delegate方法，用于支持批量请求的callback。
 */
@interface YTKBatchRequest : NSObject

///  All the requests are stored in this array.
/* lzy注170718：
 所有的请求都被存储在这个数组中。
 */
@property (nonatomic, strong, readonly) NSArray<YTKRequest *> *requestArray;

///  The delegate object of the batch request. Default is nil.
/* lzy注170718：
 批量请求的delegate对象，默认是nil。
 */
@property (nonatomic, weak, nullable) id<YTKBatchRequestDelegate> delegate;

///  The success callback. Note this will be called only if all the requests are finished.
///  This block will be called on the main queue.
/* lzy注170718：
 批量请求的成功callback。注意，这个block只会在所有请求结束之后，在主线程调用。
 */
@property (nonatomic, copy, nullable) void (^successCompletionBlock)(YTKBatchRequest *);

///  The failure callback. Note this will be called if one of the requests fails.
///  This block will be called on the main queue.
/* lzy注170718：
 批量请求失败的callback。注意，这个block将会在某个请求失败后，在主线程回调。
 */
@property (nonatomic, copy, nullable) void (^failureCompletionBlock)(YTKBatchRequest *);

///  Tag can be used to identify batch request. Default value is 0.
/* lzy注170718：
 tag值可以用于标识批量请求。默认值是0。
 */
@property (nonatomic) NSInteger tag;

///  This can be used to add several accossories object. Note if you use `addAccessory` to add acceesory
///  this array will be automatically created. Default is nil.
/* lzy注170718：
 这个数组可用于添加多个『请求配件』对象。注意，如果你使用『addAccessory』方法来添加 『请求配件』，这个数组会被自动创建，默认值是nil。
 */
@property (nonatomic, strong, nullable) NSMutableArray<id<YTKRequestAccessory>> *requestAccessories;

///  The first request that failed (and causing the batch request to fail).
/* lzy注170718：
 导致批量请求失败的 『第一个请求失败』的请求
 */
@property (nonatomic, strong, readonly, nullable) YTKRequest *failedRequest;

///  Creates a `YTKBatchRequest` with a bunch of requests.
///
///  @param requestArray requests useds to create batch request.
///
/* lzy注170718：
 传入一组请求来创建`YTKBatchRequest`批量请求
 */
- (instancetype)initWithRequestArray:(NSArray<YTKRequest *> *)requestArray;

///  Set completion callbacks
/* lzy注170718：
 设置批量请求的完成回调，包括成功、失败回调。
 */
- (void)setCompletionBlockWithSuccess:(nullable void (^)(YTKBatchRequest *batchRequest))success
                              failure:(nullable void (^)(YTKBatchRequest *batchRequest))failure;

///  Nil out both success and failure callback blocks.
/* lzy注170718：
 把成功、失败回调置空。
 */
- (void)clearCompletionBlock;

///  Convenience method to add request accessory. See also `requestAccessories`.
/* lzy注170718：
 便捷地添加 『请求配件』的方法。参看`requestAccessories`属性
 */
- (void)addAccessory:(id<YTKRequestAccessory>)accessory;

///  Append all the requests to queue.
/* lzy注170718：
 将所有的请求添加到队列中。
 */
- (void)start;

///  Stop all the requests of the batch request.
/* lzy注170718：
 停止『批量请求』中的所有请求。
 */
- (void)stop;

///  Convenience method to start the batch request with block callbacks.
/* lzy注170718：
 方便得启动一个带回调的『批量请求』
 */
- (void)startWithCompletionBlockWithSuccess:(nullable void (^)(YTKBatchRequest *batchRequest))success
                                    failure:(nullable void (^)(YTKBatchRequest *batchRequest))failure;

///  Whether all response data is from local cache.
/* lzy注170718：
 是否所有请求的返回数据都是从本地缓存中来的。
 */
- (BOOL)isDataFromCache;

@end

NS_ASSUME_NONNULL_END
