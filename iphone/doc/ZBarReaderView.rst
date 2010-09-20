ZBarReaderView Class Reference
==============================

.. class:: ZBarReaderView

   :Inherits from: :class:`UIView`

   This is a barcode reader encapsulted in a UIView.  It manages an
   :class:`AVCaptureSession` with a camera device and a
   :class:`ZBarCaptureReader`, presents the video preview and optionally
   tracks detected barcode symbols.  A delegate will usually be assigned for
   notification of new decode results.


Properties
----------

   .. member:: id<ZBarReaderViewDelegate> readerDelegate

      The delegate that will be notified of new decode results.

   .. member:: ZBarImageScanner *scanner

      Access to the image scanner is provided for configuration. (read-only)

   .. member:: BOOL tracksSymbols

      Whether to display the tracking annotation (default ``YES``).

   .. member:: BOOL allowsPinchZoom

      Enable pinch gesture recognition for manually zooming the preview/decode
      (default ``YES``).

   .. member:: NSInteger torchMode

      An :type:`AVCaptureTorchMode` value that will be applied if/when
      appropriate.  (default Auto)

   .. member:: BOOL showsFPS

      Overlay the decode frame rate on the preview to help with performance
      optimization.  This is for *debug only* and should not be set for
      production.  (default ``NO``)

   .. member:: CGFloat zoom

      Zoom scale factor applied to the video preview *and* scanCrop.  This
      value is also updated by the pinch-zoom gesture.  Valid values are in
      the range [1,2].  (default 1.25)

   .. member:: CGRect scanCrop

      The region of the video image that will be scanned, in normalized image
      coordinates.  Note that the video image is in landscape mode (default
      {{0, 0}, {1, 1}})

   .. member:: CGAffineTransform previewTransform

      Additional transform that will be applied to the video preview.  Note
      that this transform is *not* applied to scanCrop.

   .. member:: AVCaptureDevice *device

      The capture device may be manipulated or replaced.

   .. member:: AVCaptureSession *session

      Direct access to the capture session.  Warranty void if opened.
      (read-only)

   .. member:: ZBarCaptureReader *captureReader

      Direct access to the capture reader.  Warranty void if opened.
      (read-only)

   .. member:: BOOL enableCache

      :Deprecated:

      Whether to use the inter-frame consistency cache.  This should always be
      set to ``YES``.


Instance Methods
----------------

   .. describe:: - (id) initWithImageScanner:(ZBarImageScanner*)imageScanner

      :imageScanner: A pre-configured :class:`ZBarImageScanner` to use for scanning
      :Returns: The initialized :class:`ZBarReaderView`

   .. describe:: - (void) start

      Begin/resume scanning after a call to ``stop``.

   .. describe:: - (void) stop

      Stop scanning and pause the video feed.

   .. describe:: - (void) flushCache

      Flush the inter-frame consistency cache.  Any barcodes in the frame will
      be re-recognized in subsequent frames.
