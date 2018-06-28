/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SCREEN_H__INCLUDED
#define XEFIS__CORE__SCREEN_H__INCLUDED

// Standard:
#include <cstddef>
#include <atomic>
#include <optional>
#include <vector>

// Qt:
#include <QSize>
#include <QImage>
#include <QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/screen_spec.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/registration_proof.h>
#include <xefis/utility/registry.h>


namespace xf {

namespace detail {

/**
 * Additional information for each instrument needed by the Screen object,
 * such as its position on the screen.
 */
class Details
{
  public:
	std::optional<PaintRequest>	paint_request;
	QRectF						requested_position;
	std::optional<QRect>		computed_position;
	QSize						previous_size;
	std::unique_ptr<QImage>		canvas;
	std::unique_ptr<QImage>		ready_canvas;
	int							z_index { 0 };
};

} // namespace detail


/**
 * Collects instrument images and composites them onto its own area.
 */
class Screen:
	public QWidget,
	private Registry<BasicInstrument, detail::Details>,
	private Noncopyable
{
	Q_OBJECT

  public:
	using RegistrationProof = typename Registry::RegistrationProof;

  public:
	// Ctor
	explicit
	Screen (ScreenSpec const&);

	// Dtor
	~Screen();

	/**
	 * Register instrument
	 */
	RegistrationProof
	register_instrument (BasicInstrument&);

	/**
	 * Set position and size of an instrument.
	 * Values are factors, { 0, 0 } is top-left, { 1, 1 } is bottom-right.
	 */
	void
	set (BasicInstrument const&, QRectF requested_position);

	/**
	 * Set z-index for an instrument
	 */
	void
	set_z_index (BasicInstrument const&, int z_index);

	/**
	 * Return pixel density for this screen.
	 */
	si::PixelDensity
	pixel_density() const;

  protected:
	// QWidget API:
	void
	paintEvent (QPaintEvent*) override;

	// QWidget API:
	void
	resizeEvent (QResizeEvent*) override;

  private:
	/**
	 * Update screen canvas if parameters changed.
	 */
	void
	update_canvas (QSize);

	/**
	 * Request painting of all instruments on the canvas-buffer.
	 */
	void
	paint_instruments_to_buffer();

	/**
	 * Prepare canvas for an instrument.
	 * Ensure it has requested size and set it to full alpha with color black.
	 */
	void
	prepare_canvas_for_instrument (std::unique_ptr<QImage>&, QSize);

	/**
	 * Create new image suitable for screen and instrument buffers.
	 */
	QImage
	allocate_image (QSize) const;

	void
	instrument_registered (typename Registry::Disclosure&);

	void
	instrument_unregistered (typename Registry::Disclosure&);

	void
	sort_by_z_index();

  private slots:
	/**
	 * Called when next frame should be painted.
	 */
	void
	refresh();

  private:
	QTimer*						_refresh_timer;
	QImage						_canvas;
	std::vector<Disclosure*>	_z_index_sorted_disclosures;
	ScreenSpec					_screen_spec;
};

} // namespace xf

#endif
