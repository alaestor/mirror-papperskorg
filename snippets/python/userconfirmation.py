def userConfirmation(msg: str = 'Confirm', default: bool|None = None) -> bool:
  prompt = f"{msg} ({'y/n' if default is None else 'Y/n' if default else 'y/N'}): "
  while True:
  user_input = input(prompt).strip().lower()
    if user_input in ['y', 'yes']:
      return True
    elif user_input in ['n', 'no']:
      return False
    elif default is not None:
      return default
