/* original color picker by */
/* Devine Lu Linvega */
/* https://wiki.xxiivv.com/site/plan9_color.html */

/* modified by adventuresin9 */
/* added dowiz function to send rgb values to */
/* philips wiz light bulbs */
/* run program with ip address of bulb as argument */

#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <event.h>

/* Conversion */

typedef struct RgbColor {
	unsigned char r, g, b;
} RgbColor;

typedef struct HsvColor {
	unsigned char h, s, v;
} HsvColor;

static HsvColor selection;




double
ptangle(Point a, Point b)
{
	return atan2(b.y - a.y, b.x - a.x);
}

double
ptdistance(Point a, Point b)
{
	int x = a.x - b.x;
	int y = a.y - b.y;

	return sqrt(x * x + y * y);
}

Point
circlept(Point c, int r, int degrees)
{
	double rad = (double)degrees * PI / 180.0;

	c.x += cos(rad) * r;
	c.y -= sin(rad) * r;
	return c;
}

Point
getcenter(Rectangle r)
{
	return divpt(addpt(r.min, r.max), 2);
}

int
within(Point p, Rectangle r)
{
	return p.x > r.min.x && p.x < r.max.x && p.y > r.min.y && p.y < r.max.y;
}

RgbColor
hsv2rgb(HsvColor hsv)
{
	RgbColor rgb;
	unsigned char region, remainder, p, q, t;

	if(hsv.s == 0) {
		return ((RgbColor){hsv.v, hsv.v, hsv.v});
	}
	region = hsv.h / 43;
	remainder = (hsv.h - (region * 43)) * 6;
	p = (hsv.v * (255 - hsv.s)) >> 8;
	q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
	t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;
	switch(region) {
	case 0:
		rgb.r = hsv.v;
		rgb.g = t;
		rgb.b = p;
		break;
	case 1:
		rgb.r = q;
		rgb.g = hsv.v;
		rgb.b = p;
		break;
	case 2:
		rgb.r = p;
		rgb.g = hsv.v;
		rgb.b = t;
		break;
	case 3:
		rgb.r = p;
		rgb.g = q;
		rgb.b = hsv.v;
		break;
	case 4:
		rgb.r = t;
		rgb.g = p;
		rgb.b = hsv.v;
		break;
	default:
		rgb.r = hsv.v;
		rgb.g = p;
		rgb.b = q;
		break;
	}
	return rgb;
}

HsvColor
rgb2hsv(RgbColor rgb)
{
	HsvColor hsv;
	unsigned char rgbMin, rgbMax;

	rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
	rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);
	hsv.v = rgbMax;
	if(hsv.v == 0) {
		hsv.h = 0;
		hsv.s = 0;
		return hsv;
	}
	hsv.s = 255 * (double)(rgbMax - rgbMin) / hsv.v;
	if(hsv.s == 0) {
		hsv.h = 0;
		return hsv;
	}
	if(rgbMax == rgb.r)
		hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
	else if(rgbMax == rgb.g)
		hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
	else
		hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);
	return hsv;
}

unsigned int
rgb2hex(RgbColor clr)
{
	return ((clr.r & 0xFF) << 24) + ((clr.g & 0xFF) << 16) + ((clr.b & 0xFF) << 8) + (255 & 0xFF);
}

unsigned int
hsv2hex(HsvColor hsvclr)
{
	return rgb2hex(hsv2rgb(hsvclr));
}

/* Defaults */

void
lineb(Image* dst, Point p0, Point p1, Image* src, Point sp)
{
	int dx = abs(p1.x - p0.x), sx = p0.x < p1.x ? 1 : -1;
	int dy = -abs(p1.y - p0.y), sy = p0.y < p1.y ? 1 : -1;
	int err = dx + dy, e2;

	for(;;) {
		draw(dst, Rect(p0.x, p0.y, p0.x + 1, p0.y + 1), src, nil, sp);
		if(p0.x == p1.x && p0.y == p1.y)
			break;
		e2 = 2 * err;
		if(e2 >= dy) {
			err += dy;
			p0.x += sx;
		}
		if(e2 <= dx) {
			err += dx;
			p0.y += sy;
		}
	}
}

unsigned int
gradeint(int a, int b, double ratio)
{
	return (a * ratio) + (b * (1 - ratio));
}

Point
gradept(Point a, Point b, double ratio)
{
	return Pt(
	    gradeint(a.x, b.x, ratio),
	    gradeint(a.y, b.y, ratio));
}

unsigned int
gradecolor(HsvColor a, HsvColor b, double ratio)
{
	HsvColor clr = (HsvColor){
	    gradeint(a.h, b.h, ratio),
	    gradeint(a.s, b.s, ratio),
	    gradeint(a.v, b.v, ratio)};

	return hsv2hex(clr);
}

void
gradeline(Image* dst, Point p0, Point p1, HsvColor clr0, HsvColor clr1, int segs, Point sp)
{
	for(int i = 0; i < segs; i++) {
		double ratio = (double)i / segs;
		Image* clrimg = allocimage(display, Rect(0, 0, 1, 1), RGBA32, 1,
		                           gradecolor(clr0, clr1, ratio));
		lineb(dst,
		      gradept(p0, p1, ratio),
		      gradept(p0, p1, (double)(i + 1) / segs), clrimg, ZP);
		freeimage(clrimg);
	}
}

void
redraw(Image* dst)
{
	Point size = subpt(screen->r.max, screen->r.min);
	Point center = divpt(size, 2);
	Rectangle frame = (Rectangle){Pt(0, 0), size};
	int pad = 20;
	int rad = ((size.x < size.y ? size.x : size.y) / 2) - pad;
	Image* view = allocimage(display, frame, RGBA32, 1, 0x000000FF);

	/* draw ring */
	for(int i = 0; i < 180; i++) {
		Point p0 = circlept(center, rad, i * 2);
		Point p1 = circlept(center, rad, (i + 1) * 2);
		unsigned int angle = ptangle(center, p0) / PI / 2 * 255;
		int hexclr = hsv2hex((HsvColor){angle, 255, 255});
		Image* imgclr = allocimage(display, Rect(0, 0, 1, 1), RGBA32, 1, hexclr);
		lineb(view, p0, p1, imgclr, ZP);
		freeimage(imgclr);
	}

	/* draw selection */
	RgbColor selrgb = hsv2rgb(selection);
	HsvColor selhue = (HsvColor){selection.h, 255, 255};
	unsigned int selhex = rgb2hex(selrgb);
	Image* selclr = allocimage(display, Rect(0, 0, 1, 1), RGBA32, 1, selhex);
	Image* selhueclr = allocimage(display, Rect(0, 0, 1, 1), RGBA32, 1, hsv2hex(selhue));

	/* draw hue */
	double angle = (selection.h / 255.0) * -360.0;
	Point huepos = circlept(center, rad, angle);
	fillellipse(view, huepos, 2, 2, selhueclr, ZP);

	/* draw sat */
	double distance = (selection.s / 255.0) * rad;
	Point satpos = circlept(center, distance, angle);
	ellipse(view, center, distance, distance, 0, selclr, ZP);
	fillellipse(view, satpos, 2, 2, selclr, ZP);
	gradeline(view, huepos, satpos, selhue, selection, 8, ZP);

	/* collapse if window is horizontal */
	if(size.y > size.x + 2 * pad) {
		Rectangle sliderect = Rect(0, size.y - pad * 2, size.x, size.y - pad);
		draw(view, sliderect, display->black, nil, ZP);
		gradeline(view,
		          addpt(sliderect.min, Pt(pad, pad / 2)),
		          addpt(sliderect.max, Pt(-pad, -pad / 2)),
		          (HsvColor){selection.h, selection.s, 0},
		          (HsvColor){selection.h, selection.s, 255}, 16, ZP);
		Point valpos = addpt(sliderect.min, Pt((selection.v / 255.0) * (size.x - 2 * pad) + pad, pad / 2));
		fillellipse(view, valpos, 2, 2, selclr, ZP);
	}

	/* header */
	char hexstr[16];
	char rgbstr[12];
	snprint(hexstr, sizeof(hexstr), "#%02ux%02ux%02ux",
	        (selhex >> 24) & 0xFF,
	        (selhex >> 16) & 0xFF,
	        (selhex >> 8) & 0xFF);
	snprint(rgbstr, sizeof(rgbstr), "%ud,%ud,%ud",
	        selrgb.r,
	        selrgb.g,
	        selrgb.b);
	Point hexstrsize = stringsize(display->defaultfont, hexstr);
	Point rgbstrsize = stringsize(display->defaultfont, rgbstr);

	/* collapse if window is horizontal */
	if(size.y > size.x + 2 * pad) {
		Rectangle clearrect = Rect(pad, pad, size.x - pad, 2 * pad);
		draw(view, clearrect, display->black, nil, ZP);
		string(view,
		       Pt(pad, pad),
		       display->white, ZP, display->defaultfont, hexstr);
		if(hexstrsize.x + rgbstrsize.x < size.x)
			string(view,
			       Pt(size.x - pad - rgbstrsize.x, pad),
			       selclr, ZP, display->defaultfont, rgbstr);
	}

	draw(dst, screen->r, view, nil, ZP);
	flushimage(display, 1);
	freeimage(selclr);
	freeimage(selhueclr);
	freeimage(view);
}

void
select(HsvColor clr)
{
	selection.h = clr.h;
	selection.s = clr.s;
	selection.v = clr.v;
	redraw(screen);
}

void
touch(Point m)
{
	Rectangle r = screen->r;
	Point center = getcenter(r);
	int pad = 20;
	int radius = ((Dx(r) < Dy(r) ? Dx(r) : Dy(r)) / 2) - pad;
	HsvColor newsel = selection;

	if(ptdistance(center, m) > radius) {
		int width = (r.max.x - r.min.x) - pad * 2;
		Rectangle sliderect = Rect(r.min.x, r.max.y - pad * 2, r.max.x, r.max.y);
		if(within(m, sliderect)) {
			int touchx = m.x - screen->r.min.x - pad;
			double ratio = touchx / (double)width;
			newsel.v = ratio > 1 ? 255.0 : ratio < 0 ? 0 : ratio * 255.0;
		} else {
			double angle = ptangle(center, m);
			newsel.h = (int)(angle / PI / 2 * 255) % 255;
		}
	} else {
		double distance = ptdistance(center, m);
		newsel.s = (distance / radius) * 255.0;
	}
	select(newsel);
}

int
hex2int(char a, char b)
{
	a = (a <= '9') ? a - '0' : (a & 0x7) + 9;
	b = (b <= '9') ? b - '0' : (b & 0x7) + 9;
	return (a << 4) + b;
}

void
dopaste(void)
{
	char* p;
	int f;
	if((f = open("/dev/snarf", OREAD)) >= 0) {
		char body[8];
		read(f, body, 8);
		if(body[0] == '#') {
			RgbColor rgbclr = (RgbColor){
			    hex2int(body[1], body[2]),
			    hex2int(body[3], body[4]),
			    hex2int(body[5], body[6])};
			select(rgb2hsv(rgbclr));
		}
		close(f);
	}
}

void
dosnarf(void)
{
	int f;
	if((f = open("/dev/snarf", OWRITE)) >= 0) {
		unsigned int selhex = rgb2hex(hsv2rgb(selection));
		char hexstr[16];
		snprint(hexstr, sizeof(hexstr), "#%02ux%02ux%02ux",
		        (selhex >> 24) & 0xFF,
		        (selhex >> 16) & 0xFF,
		        (selhex >> 8) & 0xFF);
		write(f, hexstr, strlen(hexstr));
		close(f);
	}
}

void
dowiz(char *wizaddr)
{
	int fd;
	RgbColor rgbcolor = hsv2rgb(selection);

	fd = dial(netmkaddr(wizaddr, "udp", "38899"), nil, nil, nil);

	fprint(fd, "{\"id\":1,\"method\":\"setPilot\",\"params\":{\"r\":%d,\"g\":%d,\"b\":%d}}", rgbcolor.r, rgbcolor.g, rgbcolor.b);
	close(fd);
}




void
eresized(int new)
{
	if(new&& getwindow(display, Refnone) < 0)
		fprint(2, "can't reattach to window");
	draw(screen, screen->r, display->black, nil, ZP);
	redraw(screen);
}

void
main(int argc, char* argv[])
{
	
	Event e;
	Mouse m;
	Menu menu;
	char* mstr[] = {"Snarf", "Paste", "to bulb", "Exit",  0};
	int key;

	if(initdraw(0, 0, "Color") < 0)
		sysfatal("initdraw failed");

	/* initial color */
	selection.h = 110;
	selection.s = 120;
	selection.v = 220;

	eresized(0);
	einit(Emouse);
	menu.item = mstr;
	menu.lasthit = 0;

	redraw(screen);

	/* Break on mouse3 */
	for(;;) {
		key = event(&e);
		if(key == Emouse) {
			m = e.mouse;
			if(m.buttons & 4) {
				if(emenuhit(3, &m, &menu) == 0)
					dosnarf();
				if(emenuhit(3, &m, &menu) == 1)
					dopaste();
				if(emenuhit(3, &m, &menu) == 2)
					dowiz(argv[1]);
				if(emenuhit(3, &m, &menu) == 3)
					exits(0);
			} else if(m.buttons & 1) {
				touch(m.xy);
			}
		}
	}
}