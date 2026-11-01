from functools import wraps

# from defaulting_dataclass import defaulting_dataclass
from dataclasses import dataclass, fields, _MISSING_TYPE # noqa: E402
import logging

class Severity:
	DEBUG = logging.DEBUG
	INFO = logging.INFO
	WARNING = logging.WARNING
	ERROR = logging.ERROR
	CRITICAL = logging.CRITICAL


def _set_default_field_values(cls):
	# Taken from Jason, 2021 SO https://stackoverflow.com/a/69944614
	for f in fields(cls):
	# If there is a default and the value of the field is none we can assign a value
		if not isinstance(f.default, _MISSING_TYPE) and getattr(cls, f.name) is None:
			setattr(cls, f.name, f.default)

@dataclass
class Error:
	message: str
	severity: Severity = Severity.ERROR

	def __call__(self, *args, **kwargs):
		print("Calling {} with args {}, kwargs {}".format(self.value, args, kwargs))

	def __post_init__(self):
		_set_default_field_values(self)

@dataclass
class Config:
	log: bool = True
	log_threshold: int  = logging.WARNING
	throw: bool = True
	throw_threshold: int = logging.ERROR

	def __post_init__(self):
		_set_default_field_values(self)

def ErrorHandler(config):
	@wraps(ErrorHandler)
	def decorator(f):
		@wraps(f)
		def wrapper(*args, **kwargs):
			try:
				return f(*args, **kwargs)
			except Exception as e:
				if config.log_errors:
					print("An error occurred:", e)
				elif not isinstance(e, KeyboardInterrupt):
					raise
		return wrapper
	return decorator

# usage
# @ErrorHandler(Config(throw=false))


class MetaException(Exception):
	"""A generic exception class."""

class BadState(MetaException):
	"""
	An exception class for errors that indicate invalid program state.
	Possibly recoverable, but how the state became invalid is often unknown;
	this is possible due to unforseen side-effects, or external influence
	(such as importing and utilizing implementation details)
	"""
